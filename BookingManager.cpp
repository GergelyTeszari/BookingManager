/* BookingManager.cpp */

/* ########## includes ########## */

#include <algorithm> /* for std::find */
#include <cstddef> /* for std::size_t */
#include <ctime> /* for std::time_t */
#include <functional> /* for std::hash */
#include <map> /* for std::multimap */
#include <stdexcept> /* for std::invalid_argument */
#include <string> /* for std::string */
#include <unordered_map> /* for std::unordered_map */
#include <utility> /* for std::move */
#include <vector> /* for std::vector */

/* ########## end includes ########## */

/* ########## class declarations ########## */

class Passenger
{
public:
    explicit Passenger(std::string name)
        : name_(std::move(name)) /* Use std::move to avoid unnecessary string copies */
    {
    }

    const std::string& name() const
    {
        return name_;
    }

private:
    std::string name_;
};

class Airport
{
public:
    explicit Airport(std::string code)
        : code_(std::move(code)) /* Use std::move to avoid unnecessary string copies */
    {
    }

    const std::string& code() const
    {
        return code_;
    }

    bool operator==(const Airport& other) const noexcept
    {
        return code_ == other.code_;
    }

private:
    std::string code_;
};

class Booking
{
public:
    Booking(
        Passenger passenger,
        std::time_t departure,
        std::vector<Airport> itinerary)
        : passenger_(std::move(passenger)), /* Use std::move to avoid unnecessary copies */
          departure_(departure),
          itinerary_(std::move(itinerary)) /* Use std::move to avoid unnecessary copies */
    {
        if (itinerary_.size() < 2)
        {
            throw std::invalid_argument(
                "Itinerary must contain at least two airports");
        }
    }

    const Passenger& getPassenger() const
    {
        return passenger_;
    }

    std::time_t getDepartureTime() const
    {
        return departure_;
    }

    const std::vector<Airport>& getItinerary() const
    {
        return itinerary_;
    }

private:
    Passenger passenger_;
    std::time_t departure_;
    std::vector<Airport> itinerary_;
};

class BookingManager
{
public:
    /* Type alias for booking identifiers */
    using BookingId = std::size_t;

    /* Requirement 1: Add a new booking. */
    BookingId addBooking(Booking booking)
    {
        /* The booking ID is simply the index of the booking in the vector */
        const BookingId id = bookings_.size();

        bool bookingInserted = false;
        bool departureInserted = false;
        std::size_t insertedLegCount = 0;

        std::multimap<std::time_t, BookingId>::iterator departureIterator;
        std::vector<RouteLeg> legsToBeInserted;
        
        const std::time_t departureTime = booking.getDepartureTime();
        const auto& itinerary = booking.getItinerary();

        for (std::size_t i = 0; i + 1 < itinerary.size(); ++i)
        {
            RouteLeg leg{itinerary[i], itinerary[i + 1]};
            if (std::find(
                legsToBeInserted.begin(),
                legsToBeInserted.end(),
                leg) == legsToBeInserted.end())
            {
                legsToBeInserted.push_back(std::move(leg));
            }
        }

        try
        {
            bookings_.push_back(std::move(booking));
            bookingInserted = true;
            
            departureIterator = bookingsByDeparture_.emplace(departureTime, id);
            departureInserted = true;

            for (const auto& leg : legsToBeInserted)
            {
                bookingsByRouteLeg_[leg].push_back(id);
                ++insertedLegCount;
            }
        }
        catch (...)
        {
            for (std::size_t i = 0; i < insertedLegCount; ++i)
            {
                auto routeEntryBucket = 
                    bookingsByRouteLeg_.find(legsToBeInserted[i]);

                if (routeEntryBucket != bookingsByRouteLeg_.end())
                {
                    routeEntryBucket->second.pop_back();

                    if (routeEntryBucket->second.empty())
                    {
                        bookingsByRouteLeg_.erase(routeEntryBucket);
                    }
                }
            }
            if (departureInserted)
            {
                bookingsByDeparture_.erase(departureIterator);
            }
            if (bookingInserted)
            {
                bookings_.pop_back();
            }
            throw; 
        }

        return id;
    }

    /* Requirement 2: Select bookings departing before a given time. */
    std::vector<Booking> selectBookingsBefore(std::time_t departureTime) const
    {
        /* Returning copies keeps the result independent from the manager's lifetime. */
        std::vector<Booking> result;

        /* Use lower_bound to find the first booking that departs at or after the given time */
        const auto end = bookingsByDeparture_.lower_bound(departureTime);

        for (auto iter = bookingsByDeparture_.begin(); iter != end; ++iter)
        {
            result.push_back(bookings_.at(iter->second));
        }

        return result;
    }

    /* Requirement 3: Select bookings visiting two airports sequentially. */
    std::vector<Booking> selectBookingsVisitingTwoAirports(
        const Airport& from, const Airport& to) const
    {
        std::vector<Booking> result;

        /* Use the route-leg index for direct lookup instead of scanning all itineraries. */
        const RouteLeg requestedLeg{from, to};

        const auto routeEntryBucket = bookingsByRouteLeg_.find(requestedLeg);
        if (routeEntryBucket == bookingsByRouteLeg_.end())
        {
            return result;
        }

        /* Convert the indexed booking IDs into independent result copies. */
        for (const BookingId id : routeEntryBucket->second)
        {
            result.push_back(bookings_.at(id));
        }

        return result;
    }

private:
    /* Booking IDs are stable vector indices, because bookings are never removed */
    std::vector<Booking> bookings_;

    std::multimap<std::time_t, BookingId> bookingsByDeparture_;
    
    struct RouteLeg
    {
        Airport from;
        Airport to;

        bool operator==(const RouteLeg& other) const noexcept
        {
            return from == other.from &&
                to == other.to;
        }
    };

    struct RouteLegHash
    {
        std::size_t operator()(const RouteLeg& leg) const noexcept
        {
            /* Hash both airport codes and combine them into one value.
            The asymmetric combination makes the route direction relevant. */
            const std::size_t fromHash =
                std::hash<std::string>{}(leg.from.code());

            const std::size_t toHash =
                std::hash<std::string>{}(leg.to.code());

            return fromHash ^ (toHash << 1);
        }
    };

    /* Exact route-leg lookup: each adjacent airport pair maps to matching booking IDs. */
    std::unordered_map<
        RouteLeg,
        std::vector<BookingId>,
        RouteLegHash>
        bookingsByRouteLeg_;
};

/* ########## end class declarations ########## */

/* temporary main function for testing */
int main()
{
    return 0;
}

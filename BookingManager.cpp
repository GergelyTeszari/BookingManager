/* BookingManager.cpp */

/* ########## includes ########## */

#include "BookingManager.h"


#include <algorithm> /* for std::find */
#include <stdexcept> /* for std::invalid_argument */
#include <utility>  /* for std::move */

/* ########## end includes ########## */

/* ########## class declarations ########## */

Passenger::Passenger(std::string name)
    : name_(std::move(name))
{
}

const std::string& Passenger::name() const
{
    return name_;
}

Airport::Airport(std::string code)
    : code_(std::move(code))
{
}

const std::string& Airport::code() const
{
    return code_;
}

bool Airport::operator==(const Airport& other) const noexcept
{
    return code_ == other.code_;
}

Booking::Booking(
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

const Passenger& Booking::getPassenger() const
{
    return passenger_;
}

std::time_t Booking::getDepartureTime() const
{
    return departure_;
}

const std::vector<Airport>& Booking::getItinerary() const
{
    return itinerary_;
}

BookingManager::BookingId BookingManager::addBooking(Booking booking)
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

std::vector<Booking> BookingManager::selectBookingsBefore(std::time_t departureTime) const
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

std::vector<Booking> BookingManager::selectBookingsVisitingTwoAirports(
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

/* ########## end class declarations ########## */

/* ########## end BookingManager.cpp ########## */

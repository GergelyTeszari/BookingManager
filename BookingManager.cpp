/* BookingManager.cpp */

/* ########## includes ########## */

#include <cstddef> /* for std::size_t */
#include <ctime> /* for std::time_t */
#include <map> /* for std::multimap */
#include <stdexcept> /* for std::invalid_argument */
#include <string> /* for std::string */
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

    const std::string& Name() const
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

    const std::string& Code() const
    {
        return code_;
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
    /* TODO: Add a new booking. */
    /* TODO: Select bookings departing before a given time. */
    /* TODO: Select bookings visiting two airports sequentially. */

private:
    using BookingId = std::size_t;

    std::vector<Booking> bookings_;
    std::multimap<std::time_t, BookingId> bookingsByDeparture_;
};

/* ########## end class declarations ########## */

/* temporary main function for testing */
int main()
{
    return 0;
}

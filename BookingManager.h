/* BookingManager.h */

#pragma once

/* ########## includes ########## */

#include <cstddef> /* for std::size_t */
#include <ctime> /* for std::time_t */
#include <functional> /* for std::hash */
#include <map> /* for std::multimap */
#include <string> /* for std::string */
#include <unordered_map> /* for std::unordered_map */
#include <vector> /* for std::vector */

/* ########## end includes ########## */

/* ########## class declarations ########## */

class Passenger
{
public:
    explicit Passenger(std::string name);

    const std::string& name() const;

private:
    std::string name_;
};

class Airport
{
public:
    explicit Airport(std::string code);

    const std::string& code() const;
    bool operator==(const Airport& other) const noexcept;

private:
    std::string code_;
};

class Booking
{
public:
    Booking(
        Passenger passenger,
        std::time_t departure,
        std::vector<Airport> itinerary);

    const Passenger& getPassenger() const;


    std::time_t getDepartureTime() const;


    const std::vector<Airport>& getItinerary() const;

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
    BookingId addBooking(Booking booking);
    
    /* Requirement 2: Select bookings departing before a given time. */
    std::vector<Booking> selectBookingsBefore(std::time_t departureTime) const;


    /* Requirement 3: Select bookings visiting two airports sequentially. */
    std::vector<Booking> selectBookingsVisitingTwoAirports(
        const Airport& from, const Airport& to) const;

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

/* ########## end BookingManager.h ########## */

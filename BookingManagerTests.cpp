/* BookingManagerTests.cpp */

/* ########## includes ########## */
#include "BookingManager.h"

#include <algorithm> /* for std::any_of */
#include <iostream> /* for std::cout, std::cerr */
#include <stdexcept> /* for std::runtime_error */
#include <string> /* for std::string */

/* ########## end includes ########## */

/* ########## constants ########## */


/* 
* Actual departure times from my first international trip, converted to Unix time.
* 2024-06-10 14:40 CEST  → 2024-06-10 12:40 UTC
* 2024-06-10 20:45 CEST  → 2024-06-10 18:45 UTC
* 2024-06-23 22:55 JST   → 2024-06-23 13:55 UTC
* 2024-06-24 11:50 CEST  → 2024-06-24 09:50 UTC
*/
constexpr std::time_t budapestDepartureToFrankfurt = 1718023200;
constexpr std::time_t frankfurtDepartureToHaneda = 1718045100;
constexpr std::time_t hanedaDepartureToMunich = 1719150900;
constexpr std::time_t munichDepartureToBudapest = 1719222600;

/* ########## end constants ########## */

/* ########## test helpers ########## */

void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

bool containsPassenger(
    const std::vector<Booking>& bookings,
    const std::string& passengerName)
{
    return std::any_of(
        bookings.begin(),
        bookings.end(),
        [&](const Booking& booking)
        {
            return booking.getPassenger().name() == passengerName;
        });
}

/* ########## end test helpers ########## */

/* ########## test cases ########## */

void testDepartureCutoff()
{
    Booking booking1(
        Passenger("Greg"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA")}
    );
    Booking booking2(
        Passenger("Greg"),
        frankfurtDepartureToHaneda,
        {Airport("FRA"), Airport("HND")}
    );
    Booking booking3(
        Passenger("Greg"),
        munichDepartureToBudapest,
        {Airport("MUC"), Airport("BUD")}
    );

    BookingManager manager; 
    manager.addBooking(std::move(booking1));
    manager.addBooking(std::move(booking2));
    manager.addBooking(std::move(booking3));

    const auto result = manager.selectBookingsBefore(frankfurtDepartureToHaneda);

    expect(result.size() == 1, "Only bookings strictly before the cutoff should be returned");
    expect(result[0].getDepartureTime() == budapestDepartureToFrankfurt, "Unexpected booking returned");
}
void testBookingsWithSameDepartureTime()
{
    Booking booking1(
        Passenger("Greg"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA")}
    );
    Booking booking2(
        Passenger("Botond"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA")}
    );

    BookingManager manager; 
    manager.addBooking(std::move(booking1));
    manager.addBooking(std::move(booking2));

    const auto result = manager.selectBookingsBefore(frankfurtDepartureToHaneda);

    expect(result.size() == 2, "Bookings with duplicate departure times must both be indexed");
    expect(containsPassenger(result, "Greg"), "Booking for Greg should be returned");
    expect(containsPassenger(result, "Botond"), "Booking for Botond should be returned");
}

void testSequentialRouteInLongerItinerary()
{
    Booking booking(
        Passenger("Greg"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA"), Airport("HND")}
    );

    BookingManager manager; 
    manager.addBooking(std::move(booking));

    const auto result = manager.selectBookingsVisitingTwoAirports(Airport("FRA"), Airport("HND"));

    expect(result.size() == 1, "Sequential route in longer itinerary should match");
}

void testNonSequentialRouteDoesNotMatch()
{
    Booking booking(
        Passenger("Greg"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA"), Airport("HND")}
    );

    BookingManager manager; 
    manager.addBooking(std::move(booking));

    const auto result = manager.selectBookingsVisitingTwoAirports(Airport("BUD"), Airport("HND"));

    expect(result.empty(), "Non-sequential route should not match");
}

void testRepeatedRouteLegReturnsBookingOnce()
{
    Booking booking(
        Passenger("Greg"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA"), Airport("BUD"), Airport("FRA")}
    );

    BookingManager manager; 
    manager.addBooking(std::move(booking));

    const auto result = manager.selectBookingsVisitingTwoAirports(Airport("BUD"), Airport("FRA"));

    expect(result.size() == 1, "Repeated route leg should return the booking only once");
}

void testMissingRouteReturnsEmptyResult()
{
    Booking booking(
        Passenger("Greg"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA")}
    );

    BookingManager manager; 
    manager.addBooking(std::move(booking));

    const auto result = manager.selectBookingsVisitingTwoAirports(Airport("FRA"), Airport("HND"));

    expect(result.empty(), "Unknown route leg should return an empty result");
}

void testMultipleBookingsForSameRoute()
{
    Booking booking1(
        Passenger("Greg"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA")}
    );
    Booking booking2(
        Passenger("Botond"),
        frankfurtDepartureToHaneda,
        {Airport("BUD"), Airport("FRA")}
    );

    BookingManager manager; 
    manager.addBooking(std::move(booking1));
    manager.addBooking(std::move(booking2));

    const auto result = manager.selectBookingsVisitingTwoAirports(Airport("BUD"), Airport("FRA"));

    expect(result.size() == 2, "Multiple bookings for the same route leg should all be returned");
    expect(containsPassenger(result, "Greg"), "Booking for Greg should be returned");
    expect(containsPassenger(result, "Botond"), "Booking for Botond should be returned");
}

void testBookingWithTooShortItineraryIsRejected()
{
    bool exceptionThrown = false;

    try
    {
        Booking booking(
            Passenger("Greg"),
            budapestDepartureToFrankfurt,
            {Airport("BUD")}
        );
    }
    catch (const std::invalid_argument&)
    {
        exceptionThrown = true;
    }

    expect(
        exceptionThrown,
        "Booking with too short itinerary should be rejected");
}

void testRouteDirectionMatters()
{
    Booking booking(
        Passenger("Greg"),
        budapestDepartureToFrankfurt,
        {Airport("BUD"), Airport("FRA")}
    );

    BookingManager manager; 
    manager.addBooking(std::move(booking));

    const auto result = manager.selectBookingsVisitingTwoAirports(Airport("FRA"), Airport("BUD"));

    expect(result.empty(), "Route direction should matter");
}

/* ########## end test cases ########## */

/* ########## main ########## */

int main()
{
    try
    {

        testBookingsWithSameDepartureTime();
        testDepartureCutoff();
        testSequentialRouteInLongerItinerary();
        testNonSequentialRouteDoesNotMatch();
        testRepeatedRouteLegReturnsBookingOnce();
        testMissingRouteReturnsEmptyResult();
        testMultipleBookingsForSameRoute();
        testBookingWithTooShortItineraryIsRejected();
        testRouteDirectionMatters();

        std::cout << "All tests passed.\n";
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Test failed: " << exception.what() << '\n';
        return 1;
    }
}

/* ########## end main ########## */

/* ########## end BookingManagerTests.cpp ########## */

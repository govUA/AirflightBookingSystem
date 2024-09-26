#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <sstream>
#include <random>
#include <algorithm>

std::string generateTicketId() {
    std::string id = "T-";
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int len = 8;
    for (int i = 0; i < len; ++i) {
        id += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return id;
}

class Ticket {
private:
    std::string id;
    std::string flight;
    std::string date;
    std::string seat;
    std::string price;
    std::string username;
public:
    Ticket(const std::string &id_, const std::string &flight_, const std::string &date_, const std::string &seat_,
           const std::string &price_, const std::string &username_)
            : id(id_), flight(flight_), date(date_), seat(seat_), price(price_), username(username_) {}


    std::string getId() const {
        return id;
    }

    std::string getInfo() const {
        return '\t' + flight + ' ' + date + ' ' + seat + ' ' + price + ' ' + username + '\n';
    }

    std::string getUser() const {
        return username;
    }

    std::string getFlight() const {
        return flight;
    }

    std::string getDate() const {
        return date;
    }

    std::string getSeat() const {
        return seat;
    }

    std::string getPrice() const {
        return price;
    }
};

class Airplane {
private:
    std::string id;
    std::string date;
    std::map<std::tuple<char, int>, bool> seats;
    std::map<int, std::string> prices;
    std::map<std::string, Ticket> bookedTickets;
    std::map<std::string, std::vector<Ticket>> userTickets;
public:
    Airplane(const std::string &data) {
        std::istringstream iss(data);
        std::string delimeter = "-";
        std::string token;
        std::string seatL, seatR, price;
        int seatLN, seatRN, rows;
        char a;
        std::tuple<char, int> tuple;
        iss >> date >> id >> rows;
        while (iss >> token) {
            iss >> price;
            seatL = token.substr(0, token.find(delimeter));
            seatR = token.substr(token.find(delimeter) + 1);
            seatLN = std::stoi(seatL);
            seatRN = std::stoi(seatR);
            while (seatLN <= seatRN) {
                a = 'A';
                for (int r = 0; r < rows; r++) {
                    tuple = std::make_tuple(a, seatLN);
                    seats[tuple] = false;
                    a++;
                }
                prices[seatLN] = price;
                seatLN++;
            }

        }

    }

    std::string getAvailableSeats() {
        int counter = 0;
        std::string availableSeats = "Available Seats for Flight ";
        availableSeats += id;
        availableSeats += ":\n\n\t";
        for (auto const &[seat, availability]: seats) {
            if (!availability) {
                counter++;
                availableSeats += std::get<0>(seat);
                availableSeats += std::to_string(std::get<1>(seat));
                availableSeats += ' ';
                availableSeats += prices[std::get<1>(seat)];
                availableSeats += '\t';
                if (counter % 10 == 0) {
                    availableSeats += "\n\t";
                }
            }
        }
        return availableSeats;
    }

    std::string book(const std::string &seatLabel, const std::string &username) {
        char seatRow = seatLabel[0];
        int seatNum = std::stoi(seatLabel.substr(1));
        auto seatTuple = std::make_tuple(seatRow, seatNum);

        if (seats.find(seatTuple) == seats.end() || seats[seatTuple]) {
            std::cout << "Seat is unavailable!\n" << std::endl;
            return "";
        }

        seats[seatTuple] = true;

        std::string ticketId = generateTicketId();
        std::string price = prices[seatNum];
        Ticket newTicket(ticketId, id, date, seatLabel, price, username);
        bookedTickets.emplace(ticketId, newTicket);
        userTickets[username].push_back(newTicket);

        std::cout << "Confirmed with ID: " + ticketId + "\t\n" << std::endl;

        return ticketId;
    }

    std::string returnTicket(const std::string &ticketId) {
        if (bookedTickets.find(ticketId) == bookedTickets.end()) {
            return "Ticket not found!";
        }

        Ticket &ticket = bookedTickets.at(ticketId);
        char seatRow = ticket.getSeat()[0];
        int seatNum = std::stoi(ticket.getSeat().substr(1));
        auto seatTuple = std::make_tuple(seatRow, seatNum);
        seats[seatTuple] = false;

        std::string refundAmount = ticket.getPrice();
        std::string username = ticket.getUser();

        bookedTickets.erase(ticketId);
        auto &userTicketList = userTickets[username];
        userTicketList.erase(std::remove_if(userTicketList.begin(), userTicketList.end(),
                                            [&](const Ticket &t) { return t.getId() == ticketId; }),
                             userTicketList.end());

        return "Confirmed " + refundAmount + " refund for " + username + '\n';
    }

    std::string viewBooking(const std::string &ticketId) {
        if (bookedTickets.find(ticketId) == bookedTickets.end()) {
            return "Ticket not found\n";
        }
        return "Information for booking " + ticketId + ":\n\n" + bookedTickets.at(ticketId).getInfo();
    }

    std::string viewUserBookings(const std::string &username) {
        if (userTickets.find(username) == userTickets.end() || userTickets[username].empty()) {
            return "";
        }

        std::string result;
        for (const Ticket &ticket: userTickets[username]) {
            result += ticket.getInfo() + "\n";
        }
        return result;
    }

    std::string viewFlightBookings() {
        if (bookedTickets.empty()) {
            return "No bookings found\n";
        }
        std::string result = "All bookings for flight " + id + ' ' + date + ":\n\n";
        int counter = 0;
        for (const auto &[ticketId, ticket]: bookedTickets) {
            counter++;
            result += '\t' + ticket.getSeat() + ' ' + ticket.getUser() + ' ' + ticket.getPrice() + '\t';
            if (counter % 7 == 0) {
                result += '\n';
            }
        }
        return result + '\n';
    }
};

class FileReader {
private:
    std::vector<std::string> flights;
public:
    FileReader(const std::string &filePath) {
        std::ifstream configFile(filePath);
        std::string flight;
        if (configFile.is_open()) {
            std::getline(configFile, flight);
            while (std::getline(configFile, flight)) {
                if (!flight.empty()) {
                    flights.push_back(flight);
                }
            }
            configFile.close();
        } else {
            std::cerr << "Error opening file" << std::endl;
        }
        flights.shrink_to_fit();
    }

    std::vector<std::string> getFlights() const {
        return flights;
    }
};


int main() {
    FileReader fileReader("../config.txt");
    auto flights = fileReader.getFlights();
    return 0;
}

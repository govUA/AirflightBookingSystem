#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <sstream>
#include <random>
#include <algorithm>
#include <windows.h>

class FileRAII {
private:
    HANDLE fileHandle;
public:
    FileRAII(const char *filePath, DWORD desiredAccess, DWORD shareMode, DWORD creationDisposition) {
        fileHandle = CreateFile(filePath, desiredAccess, shareMode, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (fileHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to open file");
        }
        std::cout << "File opened successfully\n";
    }

    ~FileRAII() {
        if (fileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(fileHandle);
            std::cout << "File closed automatically\n";
        }
    }

    DWORD readData(char *buffer, DWORD numBytesToRead) {
        DWORD bytesRead;
        if (!ReadFile(fileHandle, buffer, numBytesToRead, &bytesRead, NULL)) {
            throw std::runtime_error("Failed to read from file");
        }
        return bytesRead;
    }

    DWORD writeData(const char *buffer, DWORD numBytesToWrite) {
        DWORD bytesWritten;
        if (!WriteFile(fileHandle, buffer, numBytesToWrite, &bytesWritten, NULL)) {
            throw std::runtime_error("Failed to write to file");
        }
        return bytesWritten;
    }

    FileRAII(const FileRAII &) = delete;

    FileRAII &operator=(const FileRAII &) = delete;

    FileRAII(FileRAII &&other) noexcept: fileHandle(other.fileHandle) {
        other.fileHandle = INVALID_HANDLE_VALUE;
    }

    FileRAII &operator=(FileRAII &&other) noexcept {
        if (this != &other) {
            if (fileHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(fileHandle);
            }
            fileHandle = other.fileHandle;
            other.fileHandle = INVALID_HANDLE_VALUE;
        }
        return *this;
    }
};

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

    std::string viewUserBookings(const std::string &username) const {
        if (userTickets.find(username) == userTickets.end() || userTickets.at(username).empty()) {
            return "";
        }

        std::string result;
        for (const Ticket &ticket: userTickets.at(username)) {
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
        try {
            FileRAII file(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);

            char buffer[512];
            DWORD bytesRead = 0;
            std::string fileContent;

            while ((bytesRead = file.readData(buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytesRead] = '\0';
                fileContent += buffer;
            }

            std::istringstream iss(fileContent);
            std::string line;
            std::getline(iss, line);
            while (std::getline(iss, line)) {
                if (!line.empty()) {
                    flights.push_back(line);
                }
            }

        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
        flights.shrink_to_fit();
    }

    std::vector<std::string> getFlights() const {
        return flights;
    }
};

class CLI {
private:
    std::map<std::string, Airplane> flights;
    bool running = true;

public:
    CLI(const std::vector<std::string> &flightData) {
        std::cout << "Flights:\n" << std::endl;
        for (const auto &data: flightData) {
            Airplane airplane(data);
            std::istringstream iss(data);
            std::string date, id;
            iss >> date >> id;
            flights.emplace(id + date, airplane);
            std::cout << data << std::endl;
        }
        std::cout << std::endl;
        showHelp();
    }

    void executeCommand(const std::string &command) {
        std::istringstream iss(command);
        std::string action;
        iss >> action;

        if (action == "check") {
            std::string flightDate, flightId;
            iss >> flightDate >> flightId;
            checkAvailableSeats(flightDate, flightId);

        } else if (action == "book") {
            std::string flightDate, flightId, seat, username;
            iss >> flightDate >> flightId >> seat >> username;
            bookSeat(flightDate, flightId, seat, username);

        } else if (action == "return") {
            std::string ticketId;
            iss >> ticketId;
            returnTicket(ticketId);

        } else if (action == "view") {
            std::string param1, param2;
            iss >> param1;
            if (iss >> param2) {
                if (param1 == "username") {
                    viewUserBookings(param2);
                } else {
                    viewFlightBookings(param1, param2);
                }
            } else {
                viewTicket(param1);
            }
        } else if (action == "help") {
            showHelp();

        } else if (action == "exit") {
            running = false;

        } else {
            std::cout << "Unknown command! Type 'help' for a list of available commands." << std::endl;
        }
    }

    bool isRunning() const {
        return running;
    }


private:

    void checkAvailableSeats(const std::string &flightDate, const std::string &flightId) {
        std::string key = flightId + flightDate;
        if (flights.find(key) != flights.end()) {
            std::cout << flights.at(key).getAvailableSeats() << std::endl;
        } else {
            std::cout << "Flight not found!" << std::endl;
        }
    }

    void bookSeat(const std::string &flightDate, const std::string &flightId, const std::string &seat,
                  const std::string &username) {
        std::string key = flightId + flightDate;
        if (flights.find(key) != flights.end()) {
            std::string ticketId = flights.at(key).book(seat, username);
            if (!ticketId.empty()) {
                std::cout << "Ticket booked successfully: " << ticketId << std::endl;
            }
        } else {
            std::cout << "Flight not found!" << std::endl;
        }
    }

    void returnTicket(const std::string &ticketId) {
        for (auto &[key, airplane]: flights) {
            std::string result = airplane.returnTicket(ticketId);
            if (result.find("Confirmed") != std::string::npos) {
                std::cout << result << std::endl;
                return;
            }
        }
        std::cout << "Ticket not found!" << std::endl;
    }

    void viewTicket(const std::string &ticketId) {
        for (auto &[key, airplane]: flights) {
            std::string result = airplane.viewBooking(ticketId);
            if (result.find("Ticket not found") == std::string::npos) {
                std::cout << result << std::endl;
                return;
            }
        }
        std::cout << "Ticket not found!" << std::endl;
    }

    void viewUserBookings(const std::string &username) {
        bool found = false;
        std::cout << "Bookings for user " + username + "\n" << std::endl;
        for (const auto &[key, airplane]: flights) {
            std::string result = airplane.viewUserBookings(username);
            if (!result.empty()) {
                std::cout << result << std::endl;
                found = true;
            }
        }
        if (!found) {
            std::cout << "No bookings found for user " << username << std::endl;
        }
    }

    void viewFlightBookings(const std::string &flightDate, const std::string &flightId) {
        std::string key = flightId + flightDate;
        if (flights.find(key) != flights.end()) {
            std::cout << flights.at(key).viewFlightBookings() << std::endl;
        } else {
            std::cout << "Flight not found!" << std::endl;
        }
    }

    void showHelp() {
        std::cout << "Available commands:\n";
        std::cout << "\tcheck <flightDate> <flightId>       - View available seats for a flight\n";
        std::cout << "\tbook <flightDate> <flightId> <seat> <username> - Book a seat for a user\n";
        std::cout << "\treturn <ticketId>                  - Return a ticket and refund the user\n";
        std::cout << "\tview <ticketId>                    - View details of a specific booking\n";
        std::cout << "\tview username <username>           - View all bookings for a user\n";
        std::cout << "\tview <flightDate> <flightId>       - View all bookings for a specific flight\n";
        std::cout << "\thelp                               - Show this help message\n";
        std::cout << "\texit                               - Exit the console\n\n";
    }

};


int main() {
    try {
        FileReader fileReader("../config.txt");
        auto flights = fileReader.getFlights();

        CLI cli(flights);

        std::string input;
        while (cli.isRunning() && std::getline(std::cin, input)) {
            std::cout << std::endl;
            cli.executeCommand(input);
        }

        std::cout << "Exiting the console..." << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}



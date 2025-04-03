#include "crow.h"
#include "db_connection.h"
#include <pqxx/pqxx>

// Global database instance
Database db;

int main() {
    if (!db.isConnected()) {
        std::cerr << "Failed to connect to database. Exiting..." << std::endl;
        return 1;
    }

    crow::SimpleApp app;

    // Existing route: Register user
    CROW_ROUTE(app, "/register").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string email = body["email"].s();
        std::string password = body["password"].s();

        try {
            pqxx::work txn(*db.getConnection());
            std::string query = "INSERT INTO users (username, email, password_hash) "
                                "VALUES (" + txn.quote(username) + ", " + txn.quote(email) + ", " + txn.quote(password) + ");";
            txn.exec(query);
            txn.commit();
            return crow::response(201, "User registered successfully");
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // Existing route: Login user
    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        try {
            pqxx::work txn(*db.getConnection());
            std::string query = "SELECT password_hash FROM users WHERE username = " + txn.quote(username) + ";";
            pqxx::result result = txn.exec(query);

            if (result.empty()) {
                return crow::response(400, "User not found");
            }

            std::string stored_password = result[0][0].c_str();
            if (stored_password == password) {
                return crow::response(200, "Login successful");
            } else {
                return crow::response(400, "Invalid password");
            }
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // Existing route: Edit user
    CROW_ROUTE(app, "/edit_user").methods("PUT"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string new_email = body["new_email"].s();
        std::string new_password = body["new_password"].s();

        try {
            pqxx::work txn(*db.getConnection());
            std::string query = "UPDATE users SET email = " + txn.quote(new_email) +
                                ", password_hash = " + txn.quote(new_password) +
                                " WHERE username = " + txn.quote(username) + ";";
            txn.exec(query);
            txn.commit();
            return crow::response(200, "User information updated successfully");
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // New Route: Create Auction
    CROW_ROUTE(app, "/create_auction").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        // Extract auction data from JSON
        std::string name = body["name"].s();
        std::string description = body["description"].s();
        double startingPrice = body["starting_price"].d();

        try {
            pqxx::work txn(*db.getConnection());
            std::string query =
                "INSERT INTO items (user_id, name, description, starting_price, bid_end_time) "
                "VALUES (1, " + txn.quote(name) + ", " + txn.quote(description) + ", " +
                txn.quote(startingPrice) + ", NOW() + INTERVAL '7 days')";
            txn.exec(query);
            txn.commit();

            crow::json::wvalue res;
            res["status"] = "success";
            return crow::response(201, res);
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // Basic test route
    CROW_ROUTE(app, "/")([](){
        return "Hello, Crow!";
    });

    // Start the app on port 8080 with multithreading enabled
    app.port(8080).multithreaded().run();
    return 0;
}

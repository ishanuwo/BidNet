#include "crow.h"
#include "db_connection.h"
#include <pqxx/pqxx>

Database db;

int main() {
    if (!db.isConnected()) {
        std::cerr << "Failed to connect to database. Exiting..." << std::endl;
        return 1;
    }

    crow::SimpleApp app;

    // Register route
    CROW_ROUTE(app, "/register").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string email = body["email"].s();
        std::string password = body["password"].s();

        try {
            pqxx::work txn(*db.getConnection()); // Dereference the pointer


            // Insert new user into the database
            std::string query = "INSERT INTO users (username, email, password_hash) "
                                "VALUES ('" + username + "', '" + email + "', '" + password + "');";
            txn.exec(query);  // Execute the query in the transaction

            txn.commit();  // Commit the transaction to the database
            return crow::response(201, "User registered successfully");
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // Login route
    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        try {
            pqxx::work txn(*db.getConnection()); // Dereference the pointer


            // Query to check if user exists and match the password
            std::string query = "SELECT password_hash FROM users WHERE username = '" + username + "';";
            pqxx::result result = txn.exec(query);

            if (result.empty()) {
                return crow::response(400, "User not found");
            }

            std::string stored_password = result[0][0].c_str();

            // Directly compare stored password with provided password
            if (stored_password == password) {
                return crow::response(200, "Login successful");
            } else {
                return crow::response(400, "Invalid password");
            }
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // Edit user route
    CROW_ROUTE(app, "/edit_user").methods("PUT"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string new_email = body["new_email"].s();
        std::string new_password = body["new_password"].s();

        try {
            pqxx::work txn(*db.getConnection()); // Dereference the pointer


            // Update the user's information (email and password)
            std::string query = "UPDATE users SET email = '" + new_email + "', password_hash = '" + new_password + "' "
                                "WHERE username = '" + username + "';";
            txn.exec(query);  // Execute the query in the transaction

            txn.commit();  // Commit the transaction to the database
            return crow::response(200, "User information updated successfully");
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // Basic route for testing
    CROW_ROUTE(app, "/")([](){
        return "Hello, Crow!";
    });

    // Start the app
    app.port(8080).multithreaded().run();
}

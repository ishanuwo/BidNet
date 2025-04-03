#include "crow.h"
#include "crow/middlewares/cors.h"
#include "db_connection.h"
#include <pqxx/pqxx>

// Global database instance
Database db;

int main() {
    if (!db.isConnected()) {
        std::cerr << "Failed to connect to database. Exiting..." << std::endl;
        return 1;
    }

    // Enable CORS
    crow::App<crow::CORSHandler> app;

    // Customize CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();

    // Set up CORS for all routes (you can specify more headers as needed)
    cors
      .global()
        .headers("Content-Type", "Authorization")  // Allowed headers
        .methods("POST"_method, "GET"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .max_age(5)  // Cache preflight request for 5 seconds
      .prefix("/register")
        .origin("*")  // Allow any origin for the /register route
      .prefix("/login")
        .origin("*")  // Allow any origin for the /login route
      .prefix("/edit_user")
        .origin("*")  // Allow any origin for the /edit_user route
      .prefix("/create_auction")
        .origin("*");  // Allow any origin for the /create_auction route

    // Register routes
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
        return "Check Access-Control-Allow-Methods header";
    });

    app.port(8080).run();

    return 0;
}

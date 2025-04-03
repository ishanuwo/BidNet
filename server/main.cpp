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
      .prefix("/")
        .origin("*");

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
            // Modify the query to also fetch the user_id along with the password
            std::string query = "SELECT id, password_hash FROM users WHERE username = " + txn.quote(username) + ";";
            pqxx::result result = txn.exec(query);
    
            if (result.empty()) {
                return crow::response(400, "User not found");
            }
    
            std::string stored_password = result[0][1].c_str();
            if (stored_password == password) {
                int id = result[0][0].as<int>(); // Retrieve the user_id from the result
                crow::json::wvalue res;
                res["message"] = "Login successful";
                res["id"] = id; // Return the user_id in the response
                return crow::response(200, res);
            } else {
                crow::json::wvalue res;
                res["message"] = "Invalid password";
                return crow::response(400, res);
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
        int userId = body["user_id"].i(); 
        // std::cout << "User ID: " << userId << std::endl; // Debugging line
        std::string name = body["name"].s();
        std::string description = body["description"].s();
        double startingPrice = body["starting_price"].d();

        try {
            pqxx::work txn(*db.getConnection());
            std::string query =
            "INSERT INTO items (user_id, name, description, starting_price, bid_end_time) "
            "VALUES (" + txn.quote(userId) + ", " + txn.quote(name) + ", " + txn.quote(description) + ", " +
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
        return "Hit from Crow!";
    });

    app.port(8080).run();

    return 0;
}

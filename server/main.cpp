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
    CROW_ROUTE(app, "/get_all_items").methods("GET"_method)([](const crow::request& req) {
        try {
            pqxx::work txn(*db.getConnection());

            // Query to get all items, including the 'id' field
            std::string query = "SELECT id, user_id, name, description, starting_price, bid_end_time FROM items";
            pqxx::result res = txn.exec(query);

            crow::json::wvalue response_data;
            std::vector<crow::json::wvalue> items;  // Create a vector to hold item JSONs

            // Loop through the result set and add each item to the response
            for (const auto& row : res) {
                crow::json::wvalue item;
                item["id"] = row["id"].as<int>();  // Add 'id' field
                item["user_id"] = row["user_id"].as<int>();
                item["name"] = row["name"].as<std::string>();
                item["description"] = row["description"].as<std::string>();
                item["starting_price"] = row["starting_price"].as<double>();
                item["bid_end_time"] = row["bid_end_time"].as<std::string>(); // Format as needed

                items.push_back(item); // Add the item to the vector
            }

            // Assign the vector of items to the response_data
            response_data["items"] = std::move(items);

            txn.commit();

            // Return the response with the list of items
            return crow::response(200, response_data);
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    CROW_ROUTE(app, "/get_auction_details/<string>") // Define the route with a dynamic 'id' parameter
    .methods("GET"_method)([](const crow::request& req, const std::string& id) {
        try {
            pqxx::work txn(*db.getConnection());

            // Query to fetch auction details by 'id'
            std::string query = "SELECT id, user_id, name, description, starting_price, bid_end_time FROM items WHERE id = $1";
            pqxx::result res = txn.exec_params(query, id); // Execute the query with the id parameter

            if (res.empty()) {
                return crow::response(404, "Auction not found");
            }

            crow::json::wvalue response_data;
            const auto& row = res[0]; // We expect only one row, since id is unique
            response_data["id"] = row["id"].as<int>();
            response_data["user_id"] = row["user_id"].as<int>();
            response_data["name"] = row["name"].as<std::string>();
            response_data["description"] = row["description"].as<std::string>();
            response_data["starting_price"] = row["starting_price"].as<double>();
            response_data["bid_end_time"] = row["bid_end_time"].as<std::string>(); // Assuming it's an ISO 8601 string

            txn.commit();

            // Return the auction details as JSON
            return crow::response(200, response_data);

        } catch (const std::exception& e) {
            // Handle any errors that occur
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

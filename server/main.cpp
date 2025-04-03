#include "crow.h"
#include "crow/middlewares/cors.h"
#include "db_connection.h"
#include <pqxx/pqxx>
#include <pqxx/zview>
#include <mutex>
#include <vector>
#include <algorithm>

// Global database instance
Database db;

// Global mutex for protecting bid placement operations
std::mutex bid_mutex;

// Global container for WebSocket connections
std::mutex ws_mutex;
std::vector<crow::websocket::connection*> ws_clients;

int main() {
    if (!db.isConnected()) {
        std::cerr << "Failed to connect to database. Exiting..." << std::endl;
        return 1;
    }

    // Instantiate Crow app with CORS middleware
    crow::App<crow::CORSHandler> app;

    // Configure CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
      .global()
        .headers("Content-Type", "Authorization")
        .methods("POST"_method, "GET"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .max_age(5)
      .prefix("/")
        .origin("*");

    // WebSocket route with &app
    CROW_ROUTE(app, "/ws/bids")
    .websocket(&app)
    .onopen([](crow::websocket::connection& conn) {
        std::lock_guard<std::mutex> lock(ws_mutex);
        ws_clients.push_back(&conn);
        CROW_LOG_INFO << "WebSocket opened, total clients: " << ws_clients.size();
    })
    .onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        std::lock_guard<std::mutex> lock(ws_mutex);
        auto it = std::remove(ws_clients.begin(), ws_clients.end(), &conn);
        ws_clients.erase(it, ws_clients.end());
        CROW_LOG_INFO << "WebSocket closed, total clients: " << ws_clients.size();
    })
    .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        CROW_LOG_INFO << "WebSocket received message: " << data;
    });
    // ─────────────────────────────────────────────────────────────────────────────
    // USER REGISTRATION
    // ─────────────────────────────────────────────────────────────────────────────
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
            std::string query =
                "INSERT INTO users (username, email, password_hash) "
                "VALUES (" + txn.quote(username) + ", " + txn.quote(email) + ", " + txn.quote(password) + ");";
            txn.exec(query);
            txn.commit();
            return crow::response(201, "User registered successfully");
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // USER LOGIN
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }
        std::string username = body["username"].s();
        std::string password = body["password"].s();

        try {
            pqxx::work txn(*db.getConnection());
            std::string query = "SELECT id, password_hash FROM users WHERE username = " + txn.quote(username) + ";";
            pqxx::result result = txn.exec(query);
            if (result.empty()) {
                return crow::response(400, "User not found");
            }
            std::string stored_password = result[0][1].c_str();
            if (stored_password == password) {
                int id = result[0][0].as<int>();
                crow::json::wvalue res;
                res["message"] = "Login successful";
                res["id"] = id;
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

    // ─────────────────────────────────────────────────────────────────────────────
    // EDIT USER
    // ─────────────────────────────────────────────────────────────────────────────
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
            std::string query =
                "UPDATE users SET email = " + txn.quote(new_email) +
                ", password_hash = " + txn.quote(new_password) +
                " WHERE username = " + txn.quote(username) + ";";
            txn.exec(query);
            txn.commit();
            return crow::response(200, "User information updated successfully");
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // CREATE AUCTION
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/create_auction").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }
        int userId = body["user_id"].i();
        std::string name = body["name"].s();
        std::string description = body["description"].s();
        double startingPrice = body["starting_price"].d();
        int duration = body["duration"].i(); // Duration in seconds
        std::string bidEndTimeQuery = "NOW() + INTERVAL '" + std::to_string(duration) + " seconds'";

        try {
            pqxx::work txn(*db.getConnection());
            std::string query =
                "INSERT INTO items (user_id, name, description, starting_price, current_price, bid_end_time) "
                "VALUES (" + txn.quote(userId) + ", " + txn.quote(name) + ", " + txn.quote(description) + ", " +
                txn.quote(startingPrice) + ", " + txn.quote(startingPrice) + ", " + bidEndTimeQuery + ");";
            txn.exec(query);
            txn.commit();

            crow::json::wvalue res;
            res["status"] = "success";
            return crow::response(201, res);
        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // PLACE BID (with mutex lock + WebSocket broadcast)
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/place_bid").methods("POST"_method)([](const crow::request& req) {
        // Ensure only one thread can do a bid update at a time
        std::lock_guard<std::mutex> lock(bid_mutex);

        try {
            auto bid_data = crow::json::load(req.body);
            if (!bid_data) {
                return crow::response(400, "Invalid JSON");
            }

            int user_id   = bid_data["user_id"].i();
            int seller_id = bid_data["seller_id"].i();
            int item_id   = bid_data["item_id"].i();
            double bid_amount = bid_data["bid_amount"].d();

            if (user_id <= 0 || item_id <= 0 || bid_amount <= 0) {
                return crow::response(400, "Invalid bid data");
            }

            // DB transaction for the bid
            pqxx::work txn(*db.getConnection());

            // Fetch current price
            std::string query = "SELECT current_price FROM items WHERE id = $1";
            pqxx::result res = txn.exec_params(query, item_id);
            if (res.empty()) {
                return crow::response(404, "Item not found");
            }
            double current_price = res[0]["current_price"].as<double>();

            // Check if new bid is higher
            if (bid_amount <= current_price) {
                crow::json::wvalue resp;
                resp["message"] = "Bid must be higher than the current price";
                return crow::response(400, resp);
            }

            // Insert the new bid into bids table
            std::string insert_bid =
                "INSERT INTO bids (user_id, item_id, bid_amount, seller_id) VALUES ($1, $2, $3, $4)";
            txn.exec_params(insert_bid, user_id, item_id, bid_amount, seller_id);

            // Update the current price
            std::string update_price = "UPDATE items SET current_price = $1 WHERE id = $2";
            txn.exec_params(update_price, bid_amount, item_id);

            txn.commit();

            // Broadcast to WebSocket clients
            {
                std::lock_guard<std::mutex> ws_lock(ws_mutex);
                // Construct JSON for broadcast
                std::string msg = "{\"item_id\":" + std::to_string(item_id) +
                                  ",\"new_bid\":" + std::to_string(bid_amount) + "}";
                // Send to each connected client
                for (auto client : ws_clients) {
                    client->send_text(msg);
                }
            }

            // Return success
            crow::json::wvalue out;
            out["message"]        = "Bid placed successfully";
            out["new_bid_amount"] = bid_amount;
            return crow::response(200, out);

        } catch (const std::exception &e) {
            crow::json::wvalue err;
            err["message"] = "Error: " + std::string(e.what());
            return crow::response(500, err);
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // GET ALL ITEMS
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/get_all_items").methods("GET"_method)([](const crow::request& req) {
        try {
            pqxx::work txn(*db.getConnection());
            std::string query = 
                "SELECT id, user_id, name, description, starting_price, bid_end_time, status "
                "FROM items";
            pqxx::result res = txn.exec(query);

            crow::json::wvalue response_data;
            std::vector<crow::json::wvalue> items;

            for (const auto& row : res) {
                crow::json::wvalue item;
                item["id"]             = row["id"].as<int>();
                item["user_id"]        = row["user_id"].as<int>();
                item["name"]           = row["name"].as<std::string>();
                item["description"]    = row["description"].as<std::string>();
                item["starting_price"] = row["starting_price"].as<double>();
                item["bid_end_time"]   = row["bid_end_time"].as<std::string>();
                item["status"]         = row["status"].as<std::string>();
                items.push_back(item);
            }

            response_data["items"] = std::move(items);
            txn.commit();
            return crow::response(200, response_data);

        } catch (const std::exception &e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // GET ITEM DETAILS (/<string> item_id)
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/get_item_details/<string>").methods("GET"_method)
    ([](const crow::request& req, const std::string& id) {
        try {
            pqxx::work txn(*db.getConnection());
            std::string query = 
                "SELECT id, user_id, name, description, starting_price, current_price, bid_end_time, status "
                "FROM items WHERE id = $1";
            pqxx::result res = txn.exec_params(query, id);
            if (res.empty()) {
                return crow::response(404, "Item not found");
            }
            crow::json::wvalue item;
            const auto& row = res[0];
            item["id"]             = row["id"].as<int>();
            item["user_id"]        = row["user_id"].as<int>();
            item["name"]           = row["name"].as<std::string>();
            item["description"]    = row["description"].as<std::string>();
            item["starting_price"] = row["starting_price"].as<double>();
            item["current_price"]  = row["current_price"].as<double>();
            item["bid_end_time"]   = row["bid_end_time"].as<std::string>();
            item["status"]         = row["status"].as<std::string>();
            txn.commit();
            return crow::response(200, item);
        } catch (const std::exception& e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // GET AUCTION DETAILS
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/get_auction_details/<string>").methods("GET"_method)
    ([](const crow::request& req, const std::string& id) {
        try {
            pqxx::work txn(*db.getConnection());
            std::string query =
                "SELECT id, user_id, name, description, starting_price, current_price, bid_end_time, status "
                "FROM items WHERE id = $1";
            pqxx::result res = txn.exec_params(query, id);
            if (res.empty()) {
                return crow::response(404, "Auction not found");
            }
            crow::json::wvalue auction;
            const auto& row = res[0];
            auction["id"]             = row["id"].as<int>();
            auction["user_id"]        = row["user_id"].as<int>();
            auction["name"]           = row["name"].as<std::string>();
            auction["description"]    = row["description"].as<std::string>();
            auction["starting_price"] = row["starting_price"].as<double>();
            auction["current_price"]  = row["current_price"].as<double>();
            auction["bid_end_time"]   = row["bid_end_time"].as<std::string>();
            auction["status"]         = row["status"].as<std::string>();
            txn.commit();
            return crow::response(200, auction);

        } catch (const std::exception& e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // GET TRANSACTIONS FOR BUYER
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/get_transactions_for_buyer/<int>").methods("GET"_method)
    ([](const crow::request& req, const int buyer_id) {
        try {
            pqxx::work txn(*db.getConnection());
            std::string query =
                "SELECT id, item_id, buyer_id, seller_id, final_price, transaction_time "
                "FROM transactions WHERE buyer_id = $1";
            pqxx::result res = txn.exec_params(query, buyer_id);
            if (res.empty()) {
                return crow::response(404, "No transactions found for the buyer");
            }
            crow::json::wvalue response_data;
            std::vector<crow::json::wvalue> items;
            for (const auto& row : res) {
                crow::json::wvalue t;
                t["id"]               = row["id"].as<int>();
                t["item_id"]          = row["item_id"].as<int>();
                t["buyer_id"]         = row["buyer_id"].as<int>();
                t["seller_id"]        = row["seller_id"].as<int>();
                t["final_price"]      = row["final_price"].as<double>();
                t["transaction_time"] = row["transaction_time"].as<std::string>();
                items.push_back(t);
            }
            response_data["items"] = std::move(items);
            txn.commit();
            return crow::response(200, response_data);

        } catch (const std::exception& e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // GET TRANSACTIONS FOR SELLER
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/get_transactions_for_seller/<int>").methods("GET"_method)
    ([](const crow::request& req, const int seller_id) {
        try {
            pqxx::work txn(*db.getConnection());
            std::string query =
                "SELECT id, item_id, buyer_id, seller_id, final_price, transaction_time "
                "FROM transactions WHERE seller_id = $1";
            pqxx::result res = txn.exec_params(query, seller_id);
            if (res.empty()) {
                return crow::response(404, "No transactions found for the seller");
            }
            crow::json::wvalue response_data;
            std::vector<crow::json::wvalue> items;
            for (const auto& row : res) {
                crow::json::wvalue t;
                t["id"]               = row["id"].as<int>();
                t["item_id"]          = row["item_id"].as<int>();
                t["buyer_id"]         = row["buyer_id"].as<int>();
                t["seller_id"]        = row["seller_id"].as<int>();
                t["final_price"]      = row["final_price"].as<double>();
                t["transaction_time"] = row["transaction_time"].as<std::string>();
                items.push_back(t);
            }
            response_data["items"] = std::move(items);
            txn.commit();
            return crow::response(200, response_data);

        } catch (const std::exception& e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // COMPLETE TRANSACTION
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/complete_transaction/<int>").methods("POST"_method)
    ([](const crow::request& req, const int item_id) {
        try {
            pqxx::work txn(*db.getConnection());
            // Highest bid for the item
            std::string query =
                "SELECT user_id, bid_amount, seller_id FROM bids "
                "WHERE item_id = $1 ORDER BY bid_amount DESC LIMIT 1";
            pqxx::result res = txn.exec_params(query, item_id);
            if (res.empty()) {
                return crow::response(404, "No bids found for the item");
            }
            int buyer_id   = res[0]["user_id"].as<int>();
            double bid_amt = res[0]["bid_amount"].as<double>();
            int seller_id  = res[0]["seller_id"].as<int>();

            // Insert into transactions
            std::string tx_query =
                "INSERT INTO transactions (item_id, buyer_id, seller_id, final_price, transaction_time) "
                "VALUES ($1, $2, $3, $4, NOW()) RETURNING id";
            pqxx::result tx_res = txn.exec_params(tx_query, item_id, buyer_id, seller_id, bid_amt);
            if (tx_res.empty()) {
                return crow::response(500, "Failed to create transaction");
            }
            int transaction_id = tx_res[0]["id"].as<int>();

            // Mark item as sold
            std::string update_item = "UPDATE items SET status = 'sold' WHERE id = $1";
            txn.exec_params(update_item, item_id);

            txn.commit();

            crow::json::wvalue resp;
            resp["transaction_id"]  = transaction_id;
            resp["item_id"]         = item_id;
            resp["buyer_id"]        = buyer_id;
            resp["seller_id"]       = seller_id;
            resp["final_price"]     = bid_amt;
            resp["transaction_time"] = "current time";
            return crow::response(200, resp);

        } catch (const std::exception& e) {
            return crow::response(500, "Error: " + std::string(e.what()));
        }
    });

    // ─────────────────────────────────────────────────────────────────────────────
    // BASIC TEST ROUTE
    // ─────────────────────────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/")([](){
        return "Hit from Crow!";
    });

    // Start the Crow server on port 8080
    app.port(8080).run();
    return 0;
}

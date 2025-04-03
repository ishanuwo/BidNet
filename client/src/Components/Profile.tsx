import React, { useEffect, useState } from 'react';
import { Navigate } from 'react-router-dom';
import 'bootstrap/dist/css/bootstrap.min.css';
import '../modules/Profile.css';

interface Item {
  id: number;
  user_id: number;
  name: string;
  description: string;
  starting_price: number;
  bid_end_time: string;
  status: string;
  current_price?: number;
}

interface Transaction {
  id: number;
  item_id: number;
  buyer_id: number;
  seller_id: number;
  final_price: number;
  transaction_time: string;
}

const Profile: React.FC = () => {
  const backendUrl = import.meta.env.VITE_BACKEND_URL;
  const [itemsListed, setItemsListed] = useState<Item[]>([]);
  const [itemsSold, setItemsSold] = useState<Transaction[]>([]);
  const [itemsBought, setItemsBought] = useState<Transaction[]>([]);

  // Check if a user is logged in by looking for "user" in localStorage.
  // If found, try to parse it (assuming it's stored as JSON) to extract the user id.
  const storedUser = localStorage.getItem('id');
  if (!storedUser) {
    return <Navigate to="/login" replace />;
  }

  useEffect(() => {
    fetchListedItems();
    fetchSoldTransactions();
    fetchBoughtTransactions();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const fetchListedItems = async () => {
    try {
      const res = await fetch(`${backendUrl}/get_all_items`);
      if (!res.ok) {
        throw new Error('Failed to fetch items');
      }
      const data = await res.json();
      // Filter to current user's active items.
      const userItems = data.items.filter(
        (item: Item) => item.user_id === Number(storedUser) && item.status === 'active'
      );
      // For each item, fetch additional details (current_price).
      const enrichedItems = await Promise.all(
        userItems.map(async (item: Item) => {
          const detailRes = await fetch(`${backendUrl}/get_item_details/${item.id}`);
          if (!detailRes.ok) {
            throw new Error('Failed to fetch item details');
          }
          const detailData = await detailRes.json();
          return { ...item, current_price: detailData.current_price };
        })
      );
      setItemsListed(enrichedItems);
    } catch (error) {
      console.error(error);
    }
  };

  const fetchSoldTransactions = async () => {
    try {
      const res = await fetch(`${backendUrl}/get_transactions_for_seller/${Number(storedUser)}`);
      if (!res.ok) {
        console.log('No sold transactions or server error');
        return;
      }
      const data = await res.json();
      setItemsSold(data.items);
    } catch (error) {
      console.error(error);
    }
  };

  const fetchBoughtTransactions = async () => {
    try {
      const res = await fetch(`${backendUrl}/get_transactions_for_buyer/${Number(storedUser)}`);
      if (!res.ok) {
        console.log('No bought transactions or server error');
        return;
      }
      const data = await res.json();
      setItemsBought(data.items);
    } catch (error) {
      console.error(error);
    }
  };

  // Function to handle finalizing a transaction (selling an item).
  const handleSellItem = async (itemId: number) => {
    try {
      const res = await fetch(`${backendUrl}/complete_transaction/${itemId}`, {
        method: 'POST',
      });
      if (!res.ok) {
        throw new Error('Failed to complete transaction');
      }
      // Remove sold item from listed items.
      setItemsListed((prev) => prev.filter((item) => item.id !== itemId));
      // Refresh sold transactions to reflect the new sale.
      fetchSoldTransactions();
    } catch (error) {
      console.error(error);
    }
  };

  return (
    <div className="profile-container d-flex flex-column align-items-center">
      <h1 className="profile-header">Profile Page</h1>
      <div className="content-container">
        <div className="row">
          {/* Items Listed */}
          <div className="col-md-4">
            <h3 className="section-title">Items Listed</h3>
            {itemsListed.length === 0 ? (
              <p>No active items listed.</p>
            ) : (
              itemsListed.map((item) => (
                <div className="card profile-card" key={item.id}>
                  <div className="card-body">
                    <h5 className="card-title">{item.name}</h5>
                    <p className="card-text">{item.description}</p>
                    <p className="card-text">
                      Current Price: $
                      {item.current_price
                        ? item.current_price.toFixed(2)
                        : item.starting_price.toFixed(2)}
                    </p>
                    <button
                      className="btn btn-primary"
                      onClick={() => handleSellItem(item.id)}
                    >
                      Sell
                    </button>
                  </div>
                </div>
              ))
            )}
          </div>
          {/* Items Sold */}
          <div className="col-md-4">
            <h3 className="section-title">Items Sold</h3>
            {itemsSold.length === 0 ? (
              <p>No sold items yet.</p>
            ) : (
              itemsSold.map((txn) => (
                <div className="card profile-card" key={txn.id}>
                  <div className="card-body">
                    <p className="card-text">Transaction ID: {txn.id}</p>
                    <p className="card-text">Item ID: {txn.item_id}</p>
                    <p className="card-text">
                      Final Price: ${txn.final_price.toFixed(2)}
                    </p>
                    <p className="card-text">Sold On: {txn.transaction_time}</p>
                  </div>
                </div>
              ))
            )}
          </div>
          {/* Items Bought */}
          <div className="col-md-4">
            <h3 className="section-title">Items Bought</h3>
            {itemsBought.length === 0 ? (
              <p>No purchased items yet.</p>
            ) : (
              itemsBought.map((txn) => (
                <div className="card profile-card" key={txn.id}>
                  <div className="card-body">
                    <p className="card-text">Transaction ID: {txn.id}</p>
                    <p className="card-text">Item ID: {txn.item_id}</p>
                    <p className="card-text">
                      Final Price: ${txn.final_price.toFixed(2)}
                    </p>
                    <p className="card-text">Purchased On: {txn.transaction_time}</p>
                  </div>
                </div>
              ))
            )}
          </div>
        </div>
      </div>
    </div>
  );
};

export default Profile;

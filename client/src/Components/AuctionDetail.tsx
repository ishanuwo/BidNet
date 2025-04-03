import React, { useEffect, useState, useRef } from 'react';
import { useParams } from 'react-router-dom';
import 'bootstrap/dist/css/bootstrap.min.css';

const backendUrl = import.meta.env.VITE_BACKEND_URL;

// Modify to match your actual shape
interface Auction {
  id: string;         // item_id in the DB
  user_id: number;
  item: string;
  startingPrice: number;
  currentPrice: number;
  description: string;
  endTime: string;    // ISO date string for countdown
}

const AuctionDetail: React.FC = () => {
  const { id } = useParams<{ id: string }>();
  const [auction, setAuction] = useState<Auction | null>(null);
  const [timeRemaining, setTimeRemaining] = useState<number>(0);
  const [newBid, setNewBid] = useState<number>(0);
  const [notification, setNotification] = useState<string>('');
  const userId = localStorage.getItem('id'); // Get user id from localStorage

  // We'll store the WebSocket ref so we can close it when unmounting
  const wsRef = useRef<WebSocket | null>(null);

  // ─────────────────────────────────────────────────────────────────────────────
  // 1. FETCH THE AUCTION DETAILS
  // ─────────────────────────────────────────────────────────────────────────────
  useEffect(() => {
    const fetchAuction = async () => {
      try {
        const response = await fetch(`${backendUrl}/get_auction_details/${id}`);
        if (!response.ok) {
          throw new Error('Failed to fetch auction details');
        }
        const data = await response.json();
        // Convert data to our Auction shape
        const fetchedAuction: Auction = {
          id: data.id.toString(),
          user_id: data.user_id,
          item: data.name,
          startingPrice: data.starting_price,
          currentPrice: data.current_price,
          description: data.description,
          endTime: data.bid_end_time,
        };
        setAuction(fetchedAuction);
      } catch (error) {
        console.error('Error fetching auction:', error);
      }
    };
    fetchAuction();
  }, [id]);

  // ─────────────────────────────────────────────────────────────────────────────
  // 2. SETUP A WEBSOCKET FOR REAL-TIME BIDS
  // ─────────────────────────────────────────────────────────────────────────────
  useEffect(() => {
    // Only open the WebSocket after we know the item ID
    if (!auction) return;

    // Create a new WebSocket - note the 'ws://' protocol
    const ws = new WebSocket('ws://localhost:8080/ws/bids');
    wsRef.current = ws;

    // Handle incoming messages
    ws.onmessage = (evt) => {
      try {
        const msg = JSON.parse(evt.data);
        // e.g. { "item_id": 12, "new_bid": 300 }
        if (msg.item_id && msg.new_bid) {
          // If the item_id matches THIS auction, update the currentPrice
          if (msg.item_id.toString() === auction.id) {
            setAuction((prev) =>
              prev ? { ...prev, currentPrice: msg.new_bid } : null
            );
            setNotification(`New bid: $${msg.new_bid}`);
          }
        }
      } catch (err) {
        console.error('WebSocket message parse error:', err);
      }
    };

    // Cleanup on unmount
    return () => {
      if (wsRef.current) {
        wsRef.current.close();
      }
    };
  }, [auction]);

  // ─────────────────────────────────────────────────────────────────────────────
  // 3. COUNTDOWN TIMER
  // ─────────────────────────────────────────────────────────────────────────────
  useEffect(() => {
    if (!auction) return;

    const interval = setInterval(() => {
      const now = new Date();
      // Adjust for local time if needed
      const end = new Date(auction.endTime);
      // Example: If your DB times are in UTC, you might do:
      // const offsetMs = now.getTimezoneOffset() * 60000;
      // const adjustedEndTime = new Date(end.getTime() - offsetMs);
      // Or just use 'end' if your server times are local.

      const diff = Math.max(end.getTime() - now.getTime(), 0);
      setTimeRemaining(diff);

      if (diff === 0) {
        clearInterval(interval);
        setNotification('Auction ended!');
      }
    }, 1000);

    return () => clearInterval(interval);
  }, [auction]);

  // ─────────────────────────────────────────────────────────────────────────────
  // 4. SUBMIT A NEW BID
  // ─────────────────────────────────────────────────────────────────────────────
  const handleBidSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();

    if (!userId) {
      setNotification('You need to be logged in to place a bid.');
      return;
    }
    if (!auction) {
      setNotification('Auction data not loaded yet.');
      return;
    }
    if (newBid <= auction.currentPrice) {
      setNotification('Bid must be higher than the current price.');
      return;
    }

    const bidData = {
      user_id: Number(userId),
      item_id: Number(auction.id),
      bid_amount: newBid,
      seller_id: auction.user_id,
    };

    fetch(`${backendUrl}/place_bid`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(bidData),
    })
      .then(async (response) => {
        const data = await response.json();
        if (!response.ok) {
          throw new Error(data.message || 'Error placing bid');
        }
        setNotification(data.message || 'Bid placed successfully!');
        if (data.new_bid_amount) {
          setAuction((prev) =>
            prev ? { ...prev, currentPrice: data.new_bid_amount } : null
          );
        }
      })
      .catch((error) => {
        console.error('Error placing bid:', error);
        setNotification(error.message);
      });

    setNewBid(0);
  };

  // ─────────────────────────────────────────────────────────────────────────────
  // 5. DISPLAY LOGIC
  // ─────────────────────────────────────────────────────────────────────────────
  // Format the remaining time
  const formatTime = (ms: number) => {
    const totalSeconds = Math.floor(ms / 1000);
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;
    return `${hours}h ${minutes}m ${seconds}s`;
  };

  if (!auction) {
    return <div>Loading auction details...</div>;
  }

  return (
    <div className="container mt-5">
      <div className="card p-4 shadow-sm">
        <h2 className="text-center">{auction.item}</h2>
        <p className="text-muted">{auction.description}</p>
        <p className="fw-bold">
          Starting Price: <span className="text-primary">${auction.startingPrice}</span>
        </p>
        <p className="fw-bold">
          Current Price: <span className="text-primary">${auction.currentPrice}</span>
        </p>
        <p className="text-danger">
          Time Remaining: {formatTime(timeRemaining)}
        </p>

        {/* Display notification if there's any */}
        {notification && <div className="alert alert-info">{notification}</div>}

        {/* Bid form */}
        <form onSubmit={handleBidSubmit} className="mt-3">
          <div className="input-group mb-3">
            <input
              type="number"
              className="form-control"
              placeholder="Enter your bid"
              value={newBid}
              onChange={(e) => setNewBid(Number(e.target.value))}
              required
            />
            <button type="submit" className="btn btn-success">
              Place Bid
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};

export default AuctionDetail;

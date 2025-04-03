import React, { useEffect, useState } from 'react';
import { useParams } from 'react-router-dom';
import 'bootstrap/dist/css/bootstrap.min.css';

const backendUrl = import.meta.env.VITE_BACKEND_URL;

interface Auction {
  id: string;
  item: string;
  startingPrice: number;
  currentPrice: number;
  description: string;
  endTime: string; // ISO date string for countdown
}

const AuctionDetail: React.FC = () => {
  const { id } = useParams<{ id: string }>();
  const [auction, setAuction] = useState<Auction | null>(null);
  const [timeRemaining, setTimeRemaining] = useState<number>(0);
  const [newBid, setNewBid] = useState<number>();
  const [notification, setNotification] = useState<string>('');
  const userId = localStorage.getItem('id'); // Get user id from localStorage

  // Fetch auction details from the API
  useEffect(() => {
    const fetchAuction = async () => {
      try {
        const response = await fetch(`${backendUrl}/get_auction_details/${id}`);
        if (!response.ok) {
          throw new Error('Failed to fetch auction details');
        }
        const data = await response.json();
        // Assuming the response is structured like this:
        const fetchedAuction: Auction = {
          id: data.id,
          item: data.name,
          startingPrice: data.starting_price,
          currentPrice: data.current_price,
          description: data.description,
          endTime: data.bid_end_time, // Assuming `bid_end_time` is in ISO format
        };
        setAuction(fetchedAuction);
      } catch (error) {
        console.error('Error fetching auction:', error);
      }
    };

    fetchAuction();
  }, [id]);

  // Countdown timer logic
  useEffect(() => {
    if (!auction) return;
    const interval = setInterval(() => {
      const now = new Date();
      const end = new Date(auction.endTime);
      const adjustedEndTime = new Date(end.getTime() - 4 * 60 * 60 * 1000); // Adjust for timezone
      const diff = Math.max(adjustedEndTime.getTime() - now.getTime(), 0);
      
      setTimeRemaining(diff);
      if (diff === 0) {
        clearInterval(interval);
        setNotification('Auction ended!');
      }
    }, 1000);
    return () => clearInterval(interval);
  }, [auction]);

  // Handle the bid submission
  const handleBidSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();

    if (!userId) {
      setNotification('You need to be logged in to place a bid.');
      return;
    }

    if (auction && newBid > auction.currentPrice) {
      const bidData = {
        user_id: userId,
        item_id: id,
        bid_amount: newBid,
      };
      
      // Send the bid data to your server (You can implement this logic in the backend)
      fetch(`${backendUrl}/place_bid`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(bidData),
      })
      .then(response => response.json())
      .then((data) => {
        setNotification(data.message);
        if(data.new_bid_amount){
          setAuction((prev) => prev ? { ...prev, currentPrice: data.new_bid_amount } : null);
        }
      })
      .catch((error) => {
        console.error('Error placing bid:', error);
        setNotification('Error placing bid. Please try again.');
      });

    } else {
      setNotification('Bid must be higher than the current price.');
    }

    setNewBid(0);
  };

  // Format the remaining time
  const formatTime = (milliseconds: number) => {
    const totalSeconds = Math.floor(milliseconds / 1000);
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;
    return `${hours}h ${minutes}m ${seconds}s`;
  };

  // If auction data is not available, show loading message
  if (!auction) return <div>Loading auction details...</div>;

  return (
    <div className="container mt-5">
      <div className="card p-4 shadow-sm">
        <h2 className="text-center">{auction.item}</h2>
        <p className="text-muted">{auction.description}</p>
        <p className="fw-bold">Starting Price: <span className="text-primary">${auction.startingPrice}</span></p>
        <p className="fw-bold">Current Price: <span className="text-primary">${auction.currentPrice}</span></p>
        <p className="text-danger">Time Remaining: {formatTime(timeRemaining)}</p>

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

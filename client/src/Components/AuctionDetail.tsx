import React, { useEffect, useState } from 'react';
import { useParams } from 'react-router-dom';
import '../App.css';
const backendUrl = import.meta.env.VITE_BACKEND_URL;
interface Auction {
  id: string;
  item: string;
  currentPrice: number;
  description: string;
  endTime: string; // ISO date string for countdown
}

const AuctionDetail: React.FC = () => {
  const { id } = useParams<{ id: string }>();
  const [auction, setAuction] = useState<Auction | null>(null);
  const [timeRemaining, setTimeRemaining] = useState<number>(0);
  const [newBid, setNewBid] = useState<number>(0);
  const [notification, setNotification] = useState<string>('');

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
          currentPrice: data.starting_price,
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
      const diff = Math.max(end.getTime() - now.getTime(), 0);
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
    if (auction && newBid > auction.currentPrice) {
      setAuction({ ...auction, currentPrice: newBid });
      setNotification(`Your bid of $${newBid} is now the highest!`);
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

  if (!auction) return <div>Loading auction details...</div>;

  return (
    <div className="auction-detail card">
      <h2>{auction.item}</h2>
      <p>{auction.description}</p>
      <p className="price">Current Price: ${auction.currentPrice}</p>
      <p className="countdown">Time Remaining: {formatTime(timeRemaining)}</p>
      {notification && <div className="notification">{notification}</div>}
      <form onSubmit={handleBidSubmit} className="bid-form">
        <input
          type="number"
          placeholder="Your bid"
          value={newBid}
          onChange={(e) => setNewBid(Number(e.target.value))}
          className="search-input"
        />
        <button type="submit" className="button">
          Place Bid
        </button>
      </form>
    </div>
  );
};

export default AuctionDetail;

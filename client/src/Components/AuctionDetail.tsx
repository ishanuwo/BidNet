import React, { useEffect, useState } from 'react';
import { useParams } from 'react-router-dom';
import '../App.css';

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

  // Simulated API call to fetch auction details
  useEffect(() => {
    const fakeAuction: Auction = {
      id: id || '1',
      item: 'Vintage Clock',
      currentPrice: 50,
      description: 'A classic vintage clock with intricate details.',
      endTime: new Date(Date.now() + 3600 * 1000).toISOString() // Auction ends in 1 hour
    };
    setAuction(fakeAuction);
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

  // Simulated real-time bid updates (replace with WebSocket in production)
  useEffect(() => {
    if (!auction) return;
    const interval = setInterval(() => {
      // Randomly simulate an incoming bid
      const randomIncrease = Math.random() > 0.7 ? Math.floor(Math.random() * 10) + 1 : 0;
      if (randomIncrease > 0) {
        setAuction((prev) =>
          prev ? { ...prev, currentPrice: prev.currentPrice + randomIncrease } : prev
        );
        setNotification(`New bid increased by $${randomIncrease}!`);
      }
    }, 5000);
    return () => clearInterval(interval);
  }, [auction]);

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

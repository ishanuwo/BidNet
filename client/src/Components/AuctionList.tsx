import React, { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';
import '../App.css';

interface Auction {
  id: number;
  item: string;
  currentPrice: number;
}

const AuctionList: React.FC = () => {
  const [auctions, setAuctions] = useState<Auction[]>([]);
  const [searchTerm, setSearchTerm] = useState('');

  useEffect(() => {
    // Fetch data from the server
    const fetchAuctions = async () => {
      try {
        const response = await fetch('http://localhost:8080/get_all_items');
        if (!response.ok) {
          throw new Error('Failed to fetch auctions');
        }
        const data = await response.json();
        // Assuming the response contains an array of auctions
        const fetchedAuctions = data.items.map((item: any) => ({
          id: item.id,
          item: item.name, // Change this to match the actual field from your response
          currentPrice: item.starting_price, // Assuming `starting_price` is the current price
        }));
        setAuctions(fetchedAuctions);
      } catch (error) {
        console.error('Error fetching auctions:', error);
      }
    };

    fetchAuctions();
  }, []);

  const filteredAuctions = auctions.filter(auction =>
    auction.item.toLowerCase().includes(searchTerm.toLowerCase())
  );

  return (
    <div className="auction-list card">
      <h2>Active Auctions</h2>
      <input
        type="text"
        placeholder="Search auctions..."
        value={searchTerm}
        onChange={(e) => setSearchTerm(e.target.value)}
        className="search-input"
      />
      <ul>
        {filteredAuctions.map((auction) => (
          <li key={auction.id}>
            <Link to={`/auction/${auction.id}`} className="button">
              {auction.item} - Current Price: ${auction.currentPrice}
            </Link>
          </li>
        ))}
      </ul>
    </div>
  );
};

export default AuctionList;

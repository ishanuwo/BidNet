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
    // Simulated API call to fetch auctions
    setAuctions([
      { id: 1, item: 'Vintage Clock', currentPrice: 50 },
      { id: 2, item: 'Antique Vase', currentPrice: 120 },
      { id: 3, item: 'Rare Painting', currentPrice: 300 },
    ]);
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

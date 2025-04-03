import React, { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';
import 'bootstrap/dist/css/bootstrap.min.css';
const backendUrl = import.meta.env.VITE_BACKEND_URL;
interface Auction {
  id: number;
  item: string;
  currentPrice: number;
}

const AuctionList: React.FC = () => {
  const [auctions, setAuctions] = useState<Auction[]>([]);
  const [searchTerm, setSearchTerm] = useState('');

  useEffect(() => {
    console.log(import.meta.env);
    // Fetch data from the server
    const fetchAuctions = async () => {
      try {
        const response = await fetch(`${backendUrl}/get_all_items`);
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
    <div className="container mt-5">
      <div className="card p-4 shadow-sm">
        <h2 className="text-center mb-4">Active Auctions</h2>
        <input
          type="text"
          className="form-control mb-3"
          placeholder="Search auctions..."
          value={searchTerm}
          onChange={(e) => setSearchTerm(e.target.value)}
        />
        <ul className="list-group">
          {filteredAuctions.length > 0 ? (
            filteredAuctions.map((auction) => (
              <li key={auction.id} className="list-group-item d-flex justify-content-between align-items-center">
                <Link to={`/auction/${auction.id}`} className="btn btn-outline-primary">
                  {auction.item} - ${auction.currentPrice}
                </Link>
              </li>
            ))
          ) : (
            <li className="list-group-item text-center text-muted">No auctions found</li>
          )}
        </ul>
      </div>
    </div>
  );
};

export default AuctionList;

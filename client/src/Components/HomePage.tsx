import React from 'react';
import { Link } from 'react-router-dom';
import '../App.css';

const HomePage: React.FC = () => (
  <div className="home-page">
    <header className="card-child">
      <h1>Welcome to BidNet</h1>
      <p>Your premier destination for real-time auctions!</p>
      <nav>
        <Link to="/auctions" className="button">Browse Auctions</Link>
        <Link to="/create-auction" className="button">Create Auction</Link>
        <Link to="/profile" className="button">Profile</Link>
      </nav>
    </header>
    <section>
      <div className="card-child">
        <h3>Real-Time Bidding</h3>
        <p>Experience auctions like never before with instant bid updates.</p>
      </div>
      <div className="card-child">
        <h3>Secure Transactions</h3>
        <p>Every bid and transaction is processed securely with ACID compliance.</p>
      </div>
      <div className="card-child">
        <h3>User-Friendly Interface</h3>
        <p>Our intuitive design makes bidding fun and easy.</p>
      </div>
    </section>
  </div>
);

export default HomePage;

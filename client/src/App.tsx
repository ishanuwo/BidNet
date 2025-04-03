import React from 'react';
import { BrowserRouter as Router, Routes, Route, Link } from 'react-router-dom';
import HomePage from './Components/HomePage';
import AuctionList from './Components/AuctionList';
import AuctionDetail from './Components/AuctionDetail';
import CreateAuction from './Components/CreateAuction';
import Profile from './Components/Profile';
import Login from './Components/Login';
import NotFound from './Components/NotFound';
import Register from './Components/Register';
import './App.css';

const App: React.FC = () => {
  return (
    <Router>
      <header className="main-header">
        <div className="header-container">
          <div className="logo">
            <Link to="/">BidNet</Link>
          </div>
          <nav className="main-nav">
            <ul>
              <li><Link to="/" className="nav-link">Home</Link></li>
              <li><Link to="/auctions" className="nav-link">Auctions</Link></li>
              <li><Link to="/create-auction" className="nav-link">Create Auction</Link></li>
              <li><Link to="/profile" className="nav-link">Profile</Link></li>
              <li><Link to="/login" className="nav-link">Login</Link></li>
              <li><Link to="/register" className="nav-link">Register</Link></li>
            </ul>
          </nav>
        </div>
      </header>
      <main id="root">
        <Routes>
          <Route path="/" element={<HomePage />} />
          <Route path="/auctions" element={<AuctionList />} />
          <Route path="/auction/:id" element={<AuctionDetail />} />
          <Route path="/create-auction" element={<CreateAuction />} />
          <Route path="/profile" element={<Profile />} />
          <Route path="/login" element={<Login />} />
          <Route path="/register" element={<Register />} />
          <Route path="*" element={<NotFound />} />
        </Routes>
      </main>
      <footer className="main-footer">
        <p>&copy; {new Date().getFullYear()} BidNet. All rights reserved.</p>
      </footer>
    </Router>
  );
};

export default App;

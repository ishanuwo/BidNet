import React, { useEffect, useState } from 'react';
import { BrowserRouter as Router, Routes, Route, Link, useNavigate } from 'react-router-dom';
import HomePage from './Components/HomePage';
import AuctionList from './Components/AuctionList';
import AuctionDetail from './Components/AuctionDetail';
import CreateAuction from './Components/CreateAuction';
import Login from './Components/Login';
import NotFound from './Components/NotFound';
import Register from './Components/Register';
import './App.css';

const App: React.FC = () => {
  const [isAuthenticated, setIsAuthenticated] = useState<boolean>(false);

  // Check if user is logged in when app loads
  useEffect(() => {
    const storedUser = localStorage.getItem('user');
    setIsAuthenticated(!!storedUser);
  }, []);

  const handleLogout = () => {
    localStorage.removeItem('user'); // Remove stored user info
    setIsAuthenticated(false);
  };

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

              {isAuthenticated ? (
                <li>
                  <button onClick={handleLogout} className="nav-link logout-btn">Logout</button>
                </li>
              ) : (
                <>
                  <li><Link to="/login" className="nav-link">Login</Link></li>
                  <li><Link to="/register" className="nav-link">Register</Link></li>
                </>
              )}
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
          <Route path="/login" element={<Login setIsAuthenticated={setIsAuthenticated} />} />
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

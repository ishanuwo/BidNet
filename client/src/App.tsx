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
      <div>
        {/* Navbar */}
        <nav className="navbar navbar-expand-lg navbar-light bg-white shadow-sm">
          <div className="container-fluid">
            <Link className="navbar-brand fs-3" to="/home">BidNet</Link>
            <button className="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav" aria-controls="navbarNav" aria-expanded="false" aria-label="Toggle navigation">
              <span className="navbar-toggler-icon"></span>
            </button>
            <div className="collapse navbar-collapse" id="navbarNav">
              <ul className="navbar-nav ms-auto">
                <li className="nav-item">
                  <Link className="nav-link fs-5" to="/home">Home</Link>
                </li>
                <li className="nav-item">
                  <Link className="nav-link fs-5" to="/auctions">Auctions</Link>
                </li>
                <li className="nav-item">
                  <Link className="nav-link fs-5" to="/create-auction">Create Auction</Link>
                </li>
                {!isAuthenticated ? (
                  <>
                    <li className="nav-item">
                      <Link className="nav-link fs-5" to="/login">Login</Link>
                    </li>
                    <li className="nav-item">
                      <Link className="nav-link fs-5" to="/register">Register</Link>
                    </li>
                  </>
                ) : (
                  <li className="nav-item">
                    <button className="btn btn-link nav-link fs-5" onClick={handleLogout}>Logout</button>
                  </li>
                )}
              </ul>
            </div>
          </div>
        </nav>

        {/* Routes */}
        <div className="container mt-5">
          <Routes>
            <Route path="/home" element={<HomePage />} />
            <Route path="/auctions" element={<AuctionList />} />
            <Route path="/auctions/:id" element={<AuctionDetail />} />
            <Route path="/create-auction" element={<CreateAuction />} />
            <Route path="/login" element={<Login setIsAuthenticated={setIsAuthenticated}/>} />
            <Route path="/register" element={<Register />} />
            <Route path="*" element={<NotFound />} />
          </Routes>
        </div>

        {/* Footer */}
        <footer className="bg-light text-center py-3 mt-5 shadow-sm">
          <p className="mb-0">© 2025 BidNet. All rights reserved.</p>
        </footer>
      </div>
    </Router>
  );
};

export default App;

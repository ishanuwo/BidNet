import React, { useEffect, useState } from 'react';
import { BrowserRouter as Router, Routes, Route, useNavigate } from 'react-router-dom';
import HomePage from './Components/HomePage';
import AuctionList from './Components/AuctionList';
import AuctionDetail from './Components/AuctionDetail';
import CreateAuction from './Components/CreateAuction';
import Login from './Components/Login';
import NotFound from './Components/NotFound';
import Register from './Components/Register';
import Navbar from './Components/Navbar';
import './App.css';

const App: React.FC = () => {
  const [isAuthenticated, setIsAuthenticated] = useState<boolean>(false);
  const navigate = useNavigate();

  useEffect(() => {
    const storedUser = localStorage.getItem('user');
    setIsAuthenticated(!!storedUser);
  }, []);

  const handleLogout = () => {
    localStorage.removeItem('user');
    setIsAuthenticated(false);
    navigate('/');
  };

  return (
      <div>
        <Navbar isAuthenticated={isAuthenticated} handleLogout={handleLogout} />
        <div className="container mt-1">
          <Routes>
            <Route path="/" element={<HomePage />} />
            <Route path="/auctions" element={<AuctionList />} />
            <Route path="/auction/:id" element={<AuctionDetail />} />
            <Route path="/create-auction" element={<CreateAuction />} />
            <Route path="/login" element={<Login setIsAuthenticated={setIsAuthenticated}/>} />
            <Route path="/register" element={<Register />} />
            <Route path="*" element={<NotFound />} />
          </Routes>
        </div>
        <footer className="bg-light text-center py-3 mt-5 shadow-sm">
          <p className="mb-0">© 2025 BidNet. All rights reserved.</p>
        </footer>
      </div>
  );
};

export default App;

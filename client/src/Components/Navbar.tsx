import React, { useState } from 'react';
import { Link } from 'react-router-dom';

const Navbar: React.FC<{ isAuthenticated: boolean; handleLogout: () => void }> = ({ isAuthenticated, handleLogout }) => {
  const [isOpen, setIsOpen] = useState(false);

  return (
    <nav className="navbar navbar-expand-lg navbar-light bg-white shadow-sm">
      <div className="container-fluid">
        <Link className="navbar-brand fs-3" to="/">BidNet</Link>
        <button className="navbar-toggler" type="button" onClick={() => setIsOpen(!isOpen)}>
          <span className="navbar-toggler-icon"></span>
        </button>
        <div className={`collapse navbar-collapse ${isOpen ? 'show' : ''}`} id="navbarNav">
          <ul className="navbar-nav ms-auto">
            <li className="nav-item">
              <Link className="nav-link fs-5" to="/">Home</Link>
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
  );
};

export default Navbar;

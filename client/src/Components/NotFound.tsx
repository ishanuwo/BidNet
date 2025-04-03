import React from 'react';
import { Link } from 'react-router-dom';
import '../App.css';
  

const NotFound: React.FC = () => (
  <div className="not-found card">
    <h2>404 - Page Not Found</h2>
    <p>The page you are looking for does not exist.</p>
    <Link to="/home" className="button">Return to Home</Link>
  </div>
);

export default NotFound;

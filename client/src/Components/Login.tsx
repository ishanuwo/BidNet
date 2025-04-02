import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';

import '../App.css';


const Login: React.FC = () => {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const navigate = useNavigate();

  const handleLogin = (e: React.FormEvent) => {
    e.preventDefault();
    // Simulated authentication logic
    if (username && password) {
      alert('Login successful!');
      navigate('/');
    } else {
      alert('Please enter both username and password.');
    }
  };

  return (
    <div className="login card">
      <h2>Login</h2>
      <form onSubmit={handleLogin}>
        <label>
          Username:
          <input 
            type="text" 
            value={username} 
            onChange={(e) => setUsername(e.target.value)} 
            required 
            className="search-input" 
          />
        </label>
        <br />
        <label>
          Password:
          <input 
            type="password" 
            value={password} 
            onChange={(e) => setPassword(e.target.value)} 
            required 
            className="search-input" 
          />
        </label>
        <br />
        <button type="submit" className="button">Login</button>
      </form>
    </div>
  );
};

export default Login;

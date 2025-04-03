import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';

import '../App.css';

const Register: React.FC = () => {
  const [username, setUsername] = useState('');
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const navigate = useNavigate();

  const handleRegister = async (e: React.FormEvent) => {
    e.preventDefault();

    if (username && email && password) {
      try {
        // Make the POST request to the backend register endpoint
        const response = await fetch(`http://localhost:8080/register`, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({
            username,
            email,
            password,
          }),
        });

        if (!response.ok) {
          throw new Error('Registration failed');
        }

        // If registration is successful, navigate to the login page
        alert('Registration successful!');
        navigate('/login');
      } catch (error) {
        console.error(error);
        alert('An error occurred during registration. Please try again.');
      }
    } else {
      alert('Please fill in all fields.');
    }
  };

  return (
    <div className="register card">
      <h2>Register</h2>
      <form onSubmit={handleRegister}>
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
          Email:
          <input
            type="email"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
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
        <button type="submit" className="button">Register</button>
      </form>
    </div>
  );
};

export default Register;

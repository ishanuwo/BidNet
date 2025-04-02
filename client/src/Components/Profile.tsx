import React, { useState } from 'react';
import '../App.css';

interface User {
  username: string;
  email: string;
}

//need to add credential validation and session  management so that profile only show to logged in user

const Profile: React.FC = () => {
  // Simulated user data fetch
  const [user, setUser] = useState<User>({ username: 'BidderJoe', email: 'bidderjoe@example.com' });
  const [editing, setEditing] = useState(false);
  const [newUsername, setNewUsername] = useState(user.username);
  const [newEmail, setNewEmail] = useState(user.email);

  const handleSave = () => {
    setUser({ username: newUsername, email: newEmail });
    setEditing(false);
  };

  return (
    <div className="profile card">
      <h2>Profile</h2>
      {editing ? (
        <div className="edit-profile">
          <label>
            Username:
            <input 
              type="text" 
              value={newUsername} 
              onChange={(e) => setNewUsername(e.target.value)} 
              className="search-input" 
            />
          </label>
          <br />
          <label>
            Email:
            <input 
              type="email" 
              value={newEmail} 
              onChange={(e) => setNewEmail(e.target.value)} 
              className="search-input" 
            />
          </label>
          <br />
          <button onClick={handleSave} className="button">Save</button>
        </div>
      ) : (
        <div className="profile-details">
          <p>Username: {user.username}</p>
          <p>Email: {user.email}</p>
          <button onClick={() => setEditing(true)} className="button">Edit Profile</button>
        </div>
      )}
    </div>
  );
};

export default Profile;

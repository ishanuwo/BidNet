import React, { useState, FormEvent } from 'react';
import 'bootstrap/dist/css/bootstrap.min.css';
const backendUrl = import.meta.env.VITE_BACKEND_URL;
const CreateAuction: React.FC = () => {
  const [itemName, setItemName] = useState('');
  const [description, setDescription] = useState('');
  const [startPrice, setStartPrice] = useState('');
  const [duration, setDuration] = useState(0); 

  const handleSubmit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();

    if (!itemName || !description || !startPrice) {
      alert('Please fill in all required fields.');
      return;
    }

    // Convert the starting price string to a number.
    const numericPrice = parseFloat(startPrice);
    if (isNaN(numericPrice)) {
      alert('Starting price must be a valid number.');
      return;
    }

    const data = {
      user_id: localStorage.getItem('id'),
      name: itemName,
      description,
      starting_price: numericPrice,
      duration: duration, 
    };


    try {
      const res = await fetch(`${backendUrl}/create_auction`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      });

      if (!res.ok) {
        const errorText = await res.text();
        throw new Error(`Error creating auction: ${errorText}`);
      }

      alert('Auction created successfully!');
      setItemName('');
      setDescription('');
      setStartPrice('');
      setDuration(0); 
    } catch (err: any) {
      alert(err.message);
    }
  };

  return (
    <div className="container d-flex justify-content-center align-items-center min-vh-100">
      <div className="card p-4 shadow-sm" style={{ maxWidth: '500px', width: '100%' }}>
        <h2 className="text-center mb-4">Create Auction</h2>
        <form onSubmit={handleSubmit}>
          <div className="mb-3">
            <label className="form-label">Item Name</label>
            <input
              type="text"
              className="form-control"
              value={itemName}
              onChange={(e) => setItemName(e.target.value)}
              required
            />
          </div>
          <div className="mb-3">
            <label className="form-label">Description</label>
            <textarea
              className="form-control"
              value={description}
              onChange={(e) => setDescription(e.target.value)}
              required
            />
          </div>
          <div className="mb-3">
            <label className="form-label">Starting Price ($)</label>
            <input
              type="number"
              className="form-control"
              value={startPrice}
              onChange={(e) => setStartPrice(e.target.value)}
              required
            />
          </div>
          <div className="mb-3">
            <label className="form-label">Add Duration</label>
            <input
              type="number"
              className="form-control"
              value={duration}
              onChange={(e) => setDuration(Number(e.target.value))}
              placeholder="Duration in hours"
              required
            />
          </div>
          <button type="submit" className="btn btn-primary w-100">Create Auction</button>
        </form>
      </div>
    </div>
  );
};

export default CreateAuction;

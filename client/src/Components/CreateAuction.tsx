import React, { useState, FormEvent } from 'react';
import '../App.css';

const CreateAuction: React.FC = () => {
  const [itemName, setItemName] = useState('');
  const [description, setDescription] = useState('');
  // Store the starting price as a string from the input field.
  const [startPrice, setStartPrice] = useState('');

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
      name: itemName,
      description,
      starting_price: numericPrice
    };

    console.log('Submitting data:', data);

    try {
      const res = await fetch('http://localhost:8080/create_auction', {
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
    } catch (err: any) {
      alert(err.message);
    }
  };

  return (
    <div className="create-auction card">
      <h2>Create Auction</h2>
      <form onSubmit={handleSubmit} className="auction-form">
        <label>
          Item Name:
          <input
            type="text"
            value={itemName}
            onChange={(e) => setItemName(e.target.value)}
            required
            className="search-input"
          />
        </label>
        <br />

        <label>
          Description:
          <textarea
            value={description}
            onChange={(e) => setDescription(e.target.value)}
            required
            className="search-input"
          />
        </label>
        <br />

        <label>
          Starting Price:
          <input
            type="number"
            value={startPrice}
            onChange={(e) => setStartPrice(e.target.value)}
            required
            className="search-input"
          />
        </label>
        <br />

        <button type="submit" className="button">
          Create Auction
        </button>
      </form>
    </div>
  );
};

export default CreateAuction;

import React, { useState, FormEvent } from 'react';
import '../App.css';

const CreateAuction: React.FC = () => {
  const [itemName, setItemName] = useState('');
  const [startPrice, setStartPrice] = useState<number | string>('');
  const [image, setImage] = useState<File | null>(null);
  const [preview, setPreview] = useState<string>('');

  const handleImageChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.files && e.target.files.length > 0) {
      const file = e.target.files[0];
      setImage(file);
      setPreview(URL.createObjectURL(file));
    }
  };

  const handleSubmit = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    if (!itemName || !startPrice) {
      alert('Please fill in all required fields.');
      return;
    }
    // Simulate API call to create an auction
    console.log("Creating auction:", { itemName, startPrice, image });
    alert('Auction created successfully!');
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
        <label>
          Upload Image:
          <input
            type="file"
            accept="image/*"
            onChange={handleImageChange}
            className="search-input"
          />
        </label>
        {preview && (
          <div className="image-preview">
            <img src={preview} alt="Preview" />
          </div>
        )}
        <br />
        <button type="submit" className="button">
          Create Auction
        </button>
      </form>
    </div>
  );
};

export default CreateAuction;

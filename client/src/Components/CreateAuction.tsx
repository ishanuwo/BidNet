import React, { useState, FormEvent } from 'react';
import '../App.css';

const CreateAuction: React.FC = () => {
  const [itemName, setItemName] = useState('');
  const [description, setDescription] = useState('');
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

  const handleSubmit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();

    if (!itemName || !description || !startPrice) {
      alert('Please fill in all required fields.');
      return;
    }

    // For now, just send text data as JSON. (We'll skip the image.)
    const data = {
      name: itemName,
      description,
      startPrice
      // If you want to handle images, you can do multi-part form data or a second endpoint.
    };

    try {
      const res = await fetch('http://localhost:4000/api/auction/create', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data),
      });

      if (!res.ok) {
        const errorText = await res.text();
        throw new Error(`Error creating auction: ${errorText}`);
      }

      alert('Auction created successfully!');
      setItemName('');
      setDescription('');
      setStartPrice('');
      setImage(null);
      setPreview('');
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

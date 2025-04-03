import React, { useEffect } from 'react';
import { useNavigate } from 'react-router-dom';

const NotFound: React.FC = () => {
  const navigate = useNavigate();

  useEffect(() => {
    const timer = setTimeout(() => {
      navigate('/');
    }, 2000); // Redirect after 2 seconds

    return () => clearTimeout(timer); // Cleanup timeout
  }, [navigate]);

  return (
    <div className="container text-center mt-5">
      <div className="alert alert-warning p-4 shadow-sm">
        <h2>Page Not Found</h2>
        <p>Redirecting to Home...</p>
      </div>
    </div>
  );
};

export default NotFound;

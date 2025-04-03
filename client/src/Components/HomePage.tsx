import 'bootstrap/dist/css/bootstrap.min.css';

const Homepage = () => {
  return (
    <div className="container-fluid">
      <main className="text-center">
        <div>
          <h1 className="display-4 mb-4">Welcome to BidNet</h1>
          <p className="lead mb-4">Your premier destination for real-time auctions!</p>
          <div className="row">
            <div className="col-md-4">
              <h4>Browse Auctions</h4>
              <p>Discover live auctions happening right now and place your bids.</p>
            </div>
            <div className="col-md-4">
              <h4>Create Auction</h4>
              <p>Got something to auction? Start your own auction in minutes.</p>
            </div>
            <div className="col-md-4">
              <h4>Profile</h4>
              <p>Manage your profile and track your bids with ease.</p>
            </div>
          </div>
        </div>

        <section className="py-3 bg-dark text-white">
          <div>
            <h2 className="display-5 text-center mb-4">Real-Time Bidding</h2>
            <p className="lead text-center mb-4">Experience auctions like never before with instant bid updates.</p>
            <h2 className="display-5 text-center mb-4">Secure Transactions</h2>
            <p className="lead text-center mb-4">Every bid and transaction is processed securely with ACID compliance.</p>
            <h2 className="display-5 text-center mb-4">User-Friendly Interface</h2>
            <p className="lead text-center">Our intuitive design makes bidding fun and easy.</p>
          </div>
        </section>
      </main>
    </div>
  );
};

export default Homepage;

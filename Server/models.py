from flask import Flask
from flask_sqlalchemy import SQLAlchemy
import enum

app = Flask(__name__)
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///test.db'
db = SQLAlchemy(app)

class OccupancyEnum(enum.Enum):
    absent = 0
    present = 1
    def __repr__(self):
        return self.value

class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    name = db.Column(db.String(255), nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    picture = db.Column(db.Text,unique=False,nullable=True)
    occupancy_status = db.Column(db.Enum(OccupancyEnum))
    def __repr__(self):
        return '<User %r>' % self.username
    def as_dict(self):
        return {c.name: str(getattr(self, c.name)) for c in self.__table__.columns}

class record(db.Model):
    date=db.Column(db.Date,unique=True,nullable=False)
    height=db.Column(db.Float,nullable=False)
    weight=db.Column(db.Float,nullable=False)
    actual_user=db.Column(db.String,nullable=False)
    predicted_user=db.Column(db.String,nullable=False)
    steps=db.Column(db.Integer,nullable=False)
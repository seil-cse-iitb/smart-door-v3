from models import *

db.session.add(User(username='sapan',name='Sapan Tanted', email='sapantanted@gmail.com', occupancy_status='present'))
db.session.add(User(username='shaunak',name='Shaunak Manurkar', email='shaunakmanurkar@gmail.com', occupancy_status='present'))
db.session.add(User(username='shinjan',name='Shinjan Mitra', email='shinjanxp@gmail.com', occupancy_status='absent'))
db.session.commit()
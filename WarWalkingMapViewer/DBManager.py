import sqlite3
import os

from AccessPoint import AccessPoint

NEW_DATA_DIR = 'NewData'
OLD_DATA_DIR = 'OldData'
DATABASE_DIR = 'Database'
DATABASE     = 'warwalking.sqlite3'
TABLE_NAME	 = 'warwalking'

class DBManager:

	def __init__(self):
		if not os.path.isdir(NEW_DATA_DIR):
			os.mkdir(NEW_DATA_DIR)
		if not os.path.isdir(OLD_DATA_DIR):
			os.mkdir(OLD_DATA_DIR)
		if not os.path.isdir(DATABASE_DIR):
			os.mkdir(DATABASE_DIR)

		self.dbcon = None
		self.cur = None
		self.reConnectToDB()
		self.checkDBSetup()

	def reConnectToDB(self):
		if (self.dbcon):
			self.dbcon.close()
		self.dbcon = sqlite3.connect(os.path.join(DATABASE_DIR, DATABASE))
		self.dbcon.row_factory = sqlite3.Row
		self.cur = self.dbcon.cursor()

	def collectNewData(self):
		return [f.path for f in os.scandir(NEW_DATA_DIR) if f.is_file()]

	def ingestNewData(self):

		# indices of AP struct in log files
		iBSSID=0
		iSSID=1
		iENC=2
		iRSSI=3
		iCH=4
		iLAT=5
		iLNG=6

		# optionally add GUI progress bar and optimize queries
		for filepath in self.collectNewData():
			# read file
			with open(filepath, 'r') as f:
				aps = f.readlines()
			# move file to backup location
			os.replace(filepath, os.path.join(OLD_DATA_DIR,os.path.basename(filepath)))

			# add each AP to the DB
			l = len(aps)
			n = 0
			aps = [ap.strip().split(';') for ap in aps]
			for ap in aps:
				n+=1
				self.addDBEntry(ap[iBSSID], ap[iSSID], ap[iENC], ap[iRSSI], ap[iCH], ap[iLAT], ap[iLNG])
				print("\rUploading APs to database: {0:.2f}%".format(100*(n/float(l))),end = '\r');
			print()
		self.dbcon.commit()
		self.removeRedundancies()

	def checkDBSetup(self):
		try:
			sql_select = f"""
				SELECT * FROM {TABLE_NAME}
				"""
			self.cur.execute(sql_select)
		except sqlite3.OperationalError:
			print("Created table.")
			# create table if it doesn't exist yet
			sql_create = f"""
				CREATE TABLE {TABLE_NAME} (
					id INTEGER PRIMARY KEY NOT NULL ,
					latitude REAL NOT NULL,
					longitude REAL NOT NULL,
					ssid VARCHAR(32),
					encryption VARCHAR(32),
					rssi INT,
					bssid VARCHAR(20) NOT NULL,
					channel INT
				)
				"""
			self.cur.execute(sql_create)
			self.dbcon.commit()

	def removeRedundancies(self):
		# remove any duplicate BSSID, keep the one with the best signal
		sql_rinse = f"""
				DELETE FROM {TABLE_NAME} WHERE id NOT IN 
					( SELECT l.id
						FROM 
								{TABLE_NAME} AS l
							JOIN
								(SELECT MAX(rssi) AS sig, bssid 
							 	FROM {TABLE_NAME} 
							 	GROUP BY bssid) AS r
					 		ON 
					 			r.sig=l.rssi AND r.bssid=l.bssid GROUP BY l.bssid
			 		)
				"""
		self.cur.execute(sql_rinse)
		self.dbcon.commit()

	def addDBEntry(self, bssid, ssid, enc, rssi, channel, lat, lng):
		sql_insert = f"""
			INSERT INTO {TABLE_NAME} (latitude, longitude, ssid, encryption, rssi, bssid, channel)
			VALUES (?, ?, ?, ?, ?, ?, ?)
			"""
		self.cur.execute(sql_insert, (lat, lng, ssid, enc, rssi, bssid, channel))

	def resultsToAPs(self, results):
		return [AccessPoint(
			sqlAP['latitude'],
			sqlAP['longitude'],
			sqlAP['bssid'],
			sqlAP['ssid'],
			sqlAP['encryption'],
			sqlAP['rssi'],
			sqlAP['channel'])
		for sqlAP in results]

	def addViewRestriction(self, query, view):
		if not view:
			return query
		if ('WHERE' not in query.upper()):
			return query + f'''
				WHERE (latitude BETWEEN {view["LR"][0]} AND {view["UL"][0]}) AND
				(longitude BETWEEN {view["UL"][1]} AND {view["LR"][1]}) 
			'''
		else:
			return query.replace('WHERE',f'''
			WHERE (latitude BETWEEN {view["LR"][0]} AND {view["UL"][0]}) AND
				(longitude BETWEEN {view["UL"][1]} AND {view["LR"][1]}) AND
			''')

	def getAllAPs(self, view=None):
		sql_query = f"""
			SELECT * FROM {TABLE_NAME}
		"""
		sql_query = self.addViewRestriction(sql_query, view)
		self.cur.execute(sql_query)
		results = self.cur.fetchall()
		return self.resultsToAPs(results)

	def getWEPAPs(self, view=None):
		sql_query = f"""
			SELECT * FROM {TABLE_NAME} WHERE encryption='WEP'
		"""
		sql_query = self.addViewRestriction(sql_query, view)
		self.cur.execute(sql_query)
		results = self.cur.fetchall()
		return self.resultsToAPs(results)

	def getOPENAPs(self, view=None):
		sql_query = f"""
			SELECT * FROM {TABLE_NAME} WHERE encryption='OPEN'
		"""
		sql_query = self.addViewRestriction(sql_query, view)
		self.cur.execute(sql_query)
		results = self.cur.fetchall()
		return self.resultsToAPs(results)

	def getTotalCount(self, view=None):
		sql_query = f"""
			SELECT COUNT(id) AS cnt FROM {TABLE_NAME}
		"""
		sql_query = self.addViewRestriction(sql_query, view)
		self.cur.execute(sql_query)
		results = self.cur.fetchone()
		return results['cnt']
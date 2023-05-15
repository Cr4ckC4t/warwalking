# You can simply run python WarWalkingMapViewer.py
# If there are any files in NewData - they will automatically be ingested

import tkinter
import tkintermapview  # https://github.com/TomSchimansky/TkinterMapView
from random import random

from DBManager import DBManager
from AccessPoint import AccessPoint

DEFAULT_WIDTH = 1024
DEFAULT_HEIGHT = 720
DEFAULT_ZOOM = 15
GPS_JITTER = 0.0004
PIN_COLOR = {
	'WEP' : {
		'circle': '#9B261E',
		'pin': '#C5542D'
	},
	'OTHER': {
		'circle': '#1C5F40',
		'pin': '#31B376'
	},
	'OPEN': {
		'circle': '#1C465F',
		'pin': '#206793'
	}
}


class WarWalkingMapViewer:
	def __init__(self):
		self.map_positioned = False

		self.root_tk = tkinter.Tk()
		self.root_tk.geometry(f"{DEFAULT_WIDTH}x{DEFAULT_HEIGHT}")
		self.root_tk.title("WarWalkingMapViewer")
		self.root_tk.grid()
		self.root_tk.grid_rowconfigure(0,weight=1)
		self.root_tk.grid_columnconfigure(0,weight=1)

		# create map widget
		self.map_widget = tkintermapview.TkinterMapView(self.root_tk, corner_radius=0)
		self.map_widget.grid(column=0,row=0,sticky='NESW')

		# set current widget zoom
		self.map_widget.set_zoom(DEFAULT_ZOOM)


		# add GUI elements
		sy = 120
		oy = 40
		self.ingestBtn = tkinter.Button(self.map_widget, text="Run ingestor", command=self.cb_runIngestor, bg='#333333', activebackground='#7f7f7f', highlightcolor='#7f7f7f', fg='white')
		self.ingestBtn.place(x=20,y=sy)
		self.showAllBtn = tkinter.Button(self.map_widget, text="Show all APs", command=self.cb_showAll, bg='#333333', activebackground='#7f7f7f', highlightcolor='#7f7f7f', fg='white')
		self.showAllBtn.place(x=20,y=sy+oy)
		self.showOpenBtn = tkinter.Button(self.map_widget, text="Show OPEN APs", command=self.cb_showOpen, bg='#333333', activebackground='#7f7f7f', highlightcolor='#7f7f7f', fg='white')
		self.showOpenBtn.place(x=20,y=sy+2*oy)
		self.showWEPBtn = tkinter.Button(self.map_widget, text="Show WEP APs", command=self.cb_showWEP, bg='#333333', activebackground='#7f7f7f', highlightcolor='#7f7f7f', fg='white')
		self.showWEPBtn.place(x=20,y=sy+3*oy)

		self.out = tkinter.Text(self.map_widget,bg='#333333', fg='white', height=4, width=30, state='disabled')
		self.out.place(x=20,y=sy+4*oy)

		self.apInfoTemplate = f"""AP details (click on any marker)
======={'='*32}
BSSID: <bssid>
SSID: <ssid>
Encryption: <enc>
Signal: <sig>
Channel: <ch>"""
		self.apInfo = tkinter.Text(self.map_widget,bg='#333333', fg='white', height=7, width=39, state='disabled')
		self.apInfo.place(x=20,y=sy+6*oy)
		self.setApInfo()

		# Setup DB
		self.writeOut(f'Starting database...')
		self.dbman = DBManager()
		# Process any data avilable in NewData directory
		self.writeOut(f'Running ingestor...')
		self.dbman.ingestNewData()
		self.writeOut(f'Total APs in DB: {self.dbman.getTotalCount()}')
	
		self.markers = []

		self.root_tk.mainloop()

	def setApInfo(self, ap=None):
		self.apInfo.configure(state='normal')
		self.apInfo.delete(1.0,tkinter.END)
		if not ap:
			self.apInfo.insert(tkinter.END, self.apInfoTemplate)
		else:
			i = self.apInfoTemplate.replace('<bssid>', ap.bssid)
			i = i.replace('<ssid>', ap.ssid)
			i = i.replace('<enc>', ap.enc)
			i = i.replace('<sig>', str(ap.rssi))
			i = i.replace('<ch>', str(ap.ch))
			self.apInfo.insert(tkinter.END, i)
		self.apInfo.configure(state='disabled')

	def writeOut(self,msg):
		self.out.configure(state='normal')
		self.out.see(tkinter.END)
		self.out.insert(tkinter.END, msg+'\n')
		self.out.configure(state='disabled')

	def getCurView(self):
		upLeftCoords = tkintermapview.osm_to_decimal(self.map_widget.upper_left_tile_pos[0], self.map_widget.upper_left_tile_pos[1], round(self.map_widget.zoom))
		lowRightCoords = tkintermapview.osm_to_decimal(self.map_widget.lower_right_tile_pos[0], self.map_widget.lower_right_tile_pos[1], round(self.map_widget.zoom))
		return {'UL':upLeftCoords, 'LR':lowRightCoords}

	def addMarkers(self, accessPoints):
		if len(accessPoints) > 500:
			resp = tkinter.messagebox.askyesno("Markerlimit Warning", message=f"Do you really want to create {len(accessPoints)} markers?")
			if resp == False:
				self.writeOut("Markers not updated")
				return

		# clear all previous markers
		self.clearCurMarkers()

		self.map_positioned=False
		for ap in accessPoints:
			if not self.map_positioned:
				self.map_widget.set_position(ap.lat, ap.lng)
				self.map_positioned=True
	
			self.markers.append(self.map_widget.set_marker(
				ap.lat + GPS_JITTER*random(),
				ap.lng + GPS_JITTER*random(),
				text=ap.ssid,
				marker_color_circle=(PIN_COLOR[ap.enc if ap.enc in PIN_COLOR else 'OTHER']['circle']),
				marker_color_outside=(PIN_COLOR[ap.enc if ap.enc in PIN_COLOR else 'OTHER']['pin']),
				data = ap,
				command = self.cb_marker
			))

	def clearCurMarkers(self):
		for m in self.markers:
			m.delete()

	def cb_marker(self, marker):
		self.setApInfo(marker.data)

	def cb_showAll(self):
		accessPoints = self.dbman.getAllAPs(self.getCurView())
		self.writeOut(f'Queried {len(accessPoints)} APs')
		self.addMarkers(accessPoints)

	def cb_showOpen(self):
		accessPoints = self.dbman.getOPENAPs(self.getCurView())
		self.writeOut(f'{len(accessPoints)} APs with OPEN encryption')
		self.addMarkers(accessPoints)

	def cb_showWEP(self):
		accessPoints = self.dbman.getWEPAPs(self.getCurView())
		self.writeOut(f'{len(accessPoints)} APs with WEP encryption')
		self.addMarkers(accessPoints)

	def cb_runIngestor(self):
		self.writeOut(f'Running ingestor...')
		self.dbman.ingestNewData()
		self.writeOut(f'Ingestor done')
		self.writeOut(f'Total APs in DB: {self.dbman.getTotalCount()}')

if __name__ == '__main__':
	WarWalkingMapViewer()

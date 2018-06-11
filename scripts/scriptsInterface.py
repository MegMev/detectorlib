#!/usr/bin/python

from Tkinter import *
import os
import time
import updateREST
import installation
import vars
import subprocess

print "checking if root is installed...", installation.checkinstalled("root6")
print "checking if geant4 is installed...", installation.checkinstalled("geant4")
print "checking if garfield is installed...", installation.checkinstalled("garfield")
print "checking if REST is installed...", installation.checkinstalled("REST")
print "checking if restG4 is installed...", installation.checkinstalled("restG4")
print "checking if tinyxml is installed...", installation.checkinstalled("tinyxml")

root = Tk()



class Application(Frame):
    """ Create the window and populate with widgets """

    def __init__(self,parent):
        """ initializes the frame """
        Frame.__init__(self,parent,background="white")
        self.parent = parent
        self.grid()
        self.create_widgets()
        self.isopen = 0


    def create_widgets(self):
        self.btn0 = Button(self,text="install REST",height=1,width=10)
        self.btn0.grid(row=0,column=0)
        self.btn0["command"] = self.installrestbtn

        self.btn1 = Button(self,text="install restG4",height=1,width=10)
        self.btn1.grid(row=0,column=1)
        self.btn1["command"] = self.installrestG4btn

        self.btn2 = Button(self,text="update REST",height=1,width=10)
        self.btn2.grid(row=0,column=2)
        self.btn2["command"] = self.updaterestbtn

        self.btn3 = Button(self,text="Cluster Jobs",height=1,width=10)
        self.btn3.grid(row=0,column=3)

        self.label = Label(self,textvariable=self.var,height=10,width=60)
        self.label.grid(row=1,column=0,columnspan=4)
        self.var.set("select an item to install")

        self.t = Text(self,height=1,width=30)  
        self.t.grid(row=3,column=1,columnspan=2)
        self.t.insert("1.0","input options here")

        self.btn5 = Button(self,text="previous",width=10)
        self.btn5.config(state='disabled')
        self.btn5.grid(row=3,column=0)
        self.btn5["command"] = self.stepminus

        self.btn6 = Button(self,text="next",width=10)
        self.btn6.config(state='disabled')
        self.btn6.grid(row=3,column=3)
        self.btn6["command"] = self.stepplus


    var = StringVar()


    def installrestbtn(self):
        self.step = 0
        self.refreshdisplay()

    def installrestG4btn(self):
        self.step = 10
        self.refreshdisplay()

    def updaterestbtn(self):
        self.step = 20
        self.refreshdisplay()


#define button actions
    def stepplus(self):
        self.applyopt()
        self.step = self.step + 1
        self.refreshdisplay()

    def stepminus(self):
        self.step = self.step - 1
        self.refreshdisplay()



#define step actions
    step = 0
#(0~9 REST, 10~19 restG4, <0 : install completed, 0\10\20...  : begin of the wizard)

    def refreshdisplay(self):
        if self.step == 0:
            self.t.delete("1.0",END)
            self.btn5.config(state='disabled')
            self.btn6.config(state='normal',text='next')
            self.var.set("Installing REST... Please follow this wizard")
        elif self.step == 1:
            self.t.delete("1.0",END)
            self.btn5.config(state='disabled')
            self.btn6.config(state='normal',text='next')
            if(installation.checkinstalled("REST")):
                self.var.set("REST has already been installed!\nBranch : "+updateREST.branchname()+"\nCommit : "+updateREST.commitid()+"update it or install again?(update/install)")
                vars.opt["Install_Path"] = os.environ["REST_PATH"]
                out, err = subprocess.Popen(['rest-config --flags | grep REST_WELCOME'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,shell=True).communicate()
                vars.cmakeflags[0]="-D"+out
                out, err = subprocess.Popen(['rest-config --flags | grep REST_GARFIELD'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,shell=True).communicate()
                vars.cmakeflags[1]="-D"+out
                self.t.insert("1.0","install")
            else:
                self.step = 2
                self.refreshdisplay()
        if self.step == 2:
            self.t.delete("1.0",END)
            self.btn5.config(state='disabled')
            self.btn6.config(state='normal',text='next')
            if(installation.checkinstalled("garfield")):
                self.var.set("Enable REST to use garfield library?(ON/OFF)")
                self.t.insert("1.0",vars.cmakeflags[1].split("=")[1])
            else:
                self.var.set("Garfield has not been installed, install REST anyway?(yes/no)")
                self.t.insert("1.0","yes")
        elif self.step == 3:
            self.t.delete("1.0",END)
            self.t.insert("1.0",vars.opt["Install_Path"])
            self.btn5.config(state='normal')
            self.btn6.config(state='normal',text='next')
            self.var.set("Choose an installation Path:")
        elif self.step == 4:
            self.t.delete("1.0",END)
            self.t.insert("1.0",vars.cmakeflags[0].split("=")[1])
            self.btn5.config(state='normal')
            self.btn6.config(state='normal',text='next')
            self.var.set("Enable welcome message in when logging in? (ON/OFF)")
        elif self.step == 5:
            self.t.delete("1.0",END)
            self.btn5.config(state='normal')
            self.btn6.config(state='normal',text='install')
            self.var.set("Confirm install")
        elif self.step == 10:
            self.t.delete("1.0",END)
            if(installation.checkinstalled("REST")):
                self.btn5.config(state='disabled')
                self.btn6.config(state='normal',text='install')
                self.var.set("Installing restG4, confirm?")
            else:
                self.btn5.config(state='disabled')
                self.btn6.config(state='disabled',text='install')
                self.var.set("REST mainbody has not been installed!")
        elif self.step == 20:
            if(installation.checkinstalled("REST")):
                self.t.delete("1.0",END)
                self.var.set("Local changes to project files will be overwritten!(except additions) \n Confirm update")
                self.btn5.config(state='disabled')
                self.btn6.config(state='normal',text='update')
            else:
                self.btn5.config(state='disabled')
                self.btn6.config(state='disabled',text='install')
                self.var.set("REST has not been installed!")
        elif self.step < 0:
            self.t.delete("1.0",END)
            self.btn5.config(state='disabled')
            self.btn6.config(state='disabled',text='next')

    def applyopt(self):
        if self.step == 1:
            if(installation.checkinstalled("REST")):
                if "UPDATE" in self.t.get("1.0",END).strip('\n').upper():
                    self.step = 19
                    return
                elif "INSTALL" in self.t.get("1.0",END).strip('\n').upper():
                    self.step = 1
        elif self.step == 2:
            if(installation.checkinstalled("garfield")):
                if "ON" in self.t.get("1.0",END).strip('\n').upper():
                    vars.cmakeflags[1]="-DREST_GARFIELD=ON"
                elif "OFF" in self.t.get("1.0",END).strip('\n').upper():
                    vars.cmakeflags[1]="-DREST_GARFIELD=OFF"
            else:
                if "Y" in self.t.get("1.0",END).strip('\n').upper():
                    vars.cmakeflags[1]="-DREST_GARFIELD=OFF"
                elif "N" in self.t.get("1.0",END).strip('\n').upper():
                    self.step=-10
                    self.var.set("Ended")
        elif self.step == 3:
            vars.opt["Install_Path"] = self.t.get("1.0",END).strip('\n')
        elif self.step == 4:
            vars.cmakeflags[0]="-DREST_WELCOME="+ self.t.get("1.0",END).strip('\n')
        elif self.step == 5:
            installation.install("REST")
            self.step = -10
            self.var.set("Completed! \n You need to source " + vars.opt["Install_Path"] + "/thisREST.sh before using it!")
        elif self.step == 10:
            if(installation.checkinstalled("REST")):
                installation.install("restG4")
                self.step = -10
                self.var.set("Completed!")
        elif self.step == 20:
            vars.opt["Warning"]="False"
            updateREST.main()
            self.step = 0







def main():
    root.title("REST Scripts")
    root.geometry('500x200')
    root.update()
    app = Application(root)
    root.mainloop()


if __name__ == '__main__':
	main()
import os,sys, time, commands
import subprocess, StringIO
import vars

def checkinstalled(name):
    if "root" in name:
        p = subprocess.Popen(['root-config --version'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,shell=True)
        out, err = p.communicate()
        if "6." in out and err == "":
            return True
    elif "geant4" in name:
        p = subprocess.Popen(['geant4-config --version'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,shell=True)
        out, err = p.communicate()
        if ("10." in out or "9." in out) and err == "":
            return True
    elif "garfield" in name:
        dir1 = os.environ["GARFIELD_HOME"]
        dir2 = os.environ["HEED_DATABASE"]
        dir3 = os.environ["LD_LIBRARY_PATH"]
        if dir1 != "" and dir2 != "":
            return True
    elif "REST" in name:
        p = subprocess.Popen(['rest-config --version'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,shell=True)
        out, err = p.communicate()
        if "2." in out and err == "":
            return True
    elif "restG4" in name:
            return False
    elif "tinyxml" in name:
        out1,err1 = subprocess.Popen(['find', '/usr/lib64', '-name', '*libtinyxml*'], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        out2,err2 = subprocess.Popen(['find', '/usr/lib', '-name', '*libtinyxml*'], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        out3,err3 = subprocess.Popen(['find', '/usr/local/lib64', '-name', '*libtinyxml*'], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        out4,err4 = subprocess.Popen(['find', '/usr/local/lib', '-name', '*libtinyxml*'], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        if ("tiny" in out1 and err1 == "") or ("tiny" in out2 and err2 == "") or ("tiny" in out3 and err3 == "") or ("tiny" in out4 and err4 == "") :
            return True
    else:
        print "What is ", name," ... "
    return False



def install(name):
    flag=True
    if vars.opt["Check_Installed"]=="True" or vars.opt["Check_Installed"]=="true" or vars.opt["Check_Installed"]=="1":
        if checkinstalled(name):
            print name,"has already been installed!"
            flag=False
    if flag:
        if "REST" in name:
            if checkinstalled("root")==False:
                print "ROOT6 is not installed!"
                return
            if checkinstalled("tinyxml")==False:
                print "Tinyxml is not installed!"
                return
            print "installing REST...\n\n"
            os.system("mkdir -p "+vars.opt["Build_Path"] )
            os.chdir(vars.opt["Build_Path"])
            os.system("cmake "+vars.opt["Source_Path"] +" -DINSTALL_PREFIX=" +vars.opt["Install_Path"] + " -DREST_WELCOME="+vars.opt["DREST_WELCOME"])
            os.system("make -j"+vars.opt["Make_Threads"] )
            os.system("make install")
        elif "restG4" in name:
            if checkinstalled("REST")==False :
                print "you must install rest mainbody first!"
                return
            if checkinstalled("geant4")==False :
                print "geant4 is not installed!"
                return
            print "installing restG4...\n\n"            
            os.system("mkdir -p "+vars.opt["Build_Path"]+"/restG4" )
            os.chdir(vars.opt["Build_Path"]+"/restG4")
            os.system("cmake "+vars.opt["Source_Path"]+"/packages/restG4/" +" -DINSTALL_PREFIX=" +vars.opt["Install_Path"]+"/libs/")
            os.system("make -j"+vars.opt["Make_Threads"] )
            os.system("make install")
        else:
            print "I cannot install ",name
                






def main():
    if len(sys.argv)<2:
        print "Usage: ",argv[0]," software_name(REST or restG4) [opt1=aaa] [opt2=bbb] ..."
        print "options: Check_Installed(True or False), Install_Path, Build_Path, Make_Threads(a number, 1~8)"
    else :
        if len(sys.argv)>2:
            for i in range(2, len(sys.argv)):
                vars.opt[str(sys.argv[i]).split('=')[0]]=sys.argv[i].split('=')[1]
                print vars.opt[sys.argv[i].split('=')[0]]
        name=sys.argv[1]
        install(name)



        
if __name__ == '__main__':
	main()
#coding=utf-8

'''
'''

import os,sqlite3,codecs,shutil,re

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

#分隔符
smallSplit = "******"
largeSplit = "******************"



def usage():
    print 'usage:   python fileName.py workDir'


class netEaseMailMaster(object):
    def __init__(self,workDir):
        self.workDir = workDir
        self.contactDB = '\\'.join(workDir.split('\\')[:-2]) + "\\contacts.db"
        self.searchDB = workDir +"search.db"
        self.mailDB =  workDir +"mail.db"
        self.attachFileDir = workDir +"parts"
        self.userName  = re.compile(r'[0-9a-zA-Z.]+@[0-9a-zA-Z.]+?com').findall(workDir)[0]
        self.appDB =  '\\'.join(workDir.split('\\')[:-2]) + "\\app.db"
        
    def printInfo(self):
        print "self.workDir = ",self.workDir
        print "self.contactDB",self.contactDB
        print "self.searchDB",self.searchDB
        print "self.attachFileDir",self.attachFileDir
        print "self.userName ",self.userName 

    def moveFileto(self,sourceDir, targetDir): 
        shutil.copy(sourceDir, targetDir)



    #获得邮件信息详细
    def getMailInfo(self):
        # get mail info
        # input: search.db path
        # output: (id,c0, c1, c2, c5, c6) (id,主题,发件人,收件人,邮件正文,附带文件列表)
        # 
        conn =  sqlite3.connect(self.searchDB)
        cursor = conn.execute("SELECT id,c0, c1, c2, c5, c6  from Search_content ")
        rows = cursor.fetchall()
        conn.close()
        if len(rows):
            for row in rows:
                #没有附件
                if row[-1] == u'':
                    print row
                else:
                    #提取附件 可能有多个
                    print 'there are files here',row[-1].split('\n')
                    #raw_input('print anykey to go on')
                    pass
        return rows

    #获取附件信息 并做拷贝处理
    def getAttachInfo(self):
        # get attach info
        # input: mailDB.db path
        # output: (Name,ContentType,LocalPath)
        # copy attach files
        conn =  sqlite3.connect(self.mailDB)
        cursor = conn.execute("SELECT Name,MailId,LocalPath from MailAttachment")
        rows = cursor.fetchall()
        conn.close()
        if len(rows):
            for row in rows:
                mailId = row[-2]
                sourceFileName = self.attachFileDir + row[-1]
                fileType = row[1]
                targetFileName = self.attachFileDir + "\\" + str(mailId) + "_" + row[0] 
                #print sourceFileName,">>>",targetFileName
                if len(row[-1]):
                    #copy
                    print sourceFileName,">>>",targetFileName
                    self.moveFileto(sourceFileName,targetFileName)
                print row
        return rows
    #将所有邮件内容信息写入一个文件
    def writeMailInfoToFile(self,mailInfoList):
        #mailInfo = getMailInfo(self.searchDB)
        f = codecs.open("%s-MailInfo.txt" % (self.userName),"w","utf-8")
        for eachMail in mailInfoList:
            for eachInfo in eachMail:
                text = unicode(str(eachInfo)+'\n'+smallSplit+"\n", "utf-8")
                f.write(text)
            f.write(largeSplit+u'\n')
    # write to single file FileName: MailId.html
    def writeMailInfoToSingleFile(self,mailInfoList):
        #mailInfo = getMailInfo(self.searchDB)
        for eachMail in mailInfoList:
            fileName = str(eachMail[0]) + ".txt"
            f = codecs.open(fileName,"w","utf-8")
            for eachInfo in eachMail[1:]:
                text = unicode(str(eachInfo)+'\n'+largeSplit+'\n', "utf-8")
                f.write(text)
            f.close()
            
    #获取通讯录联系人信息  需要更改以处理多用户的情况 通过AccountId区分
    def getContactsInfo(self):
        # get contact info
        # input: contacts.db path
        # output: (id,NameIndex,Name,Email)
        # 
        
        #获得当前用户的id
        
        con  = sqlite3.connect(self.appDB)
        getIdCmd = "select ID from Account where Name='%s'" % (self.userName)
        cursor = con.execute(getIdCmd)
        rows = cursor.fetchall()
        con.close()
        id = str(rows[0][0])
        
        conn =  sqlite3.connect(self.contactDB)
        cursor = conn.execute("SELECT CID,NameIndex,Name,Email from contact_view where AccountId=%s" % id)
        rows = cursor.fetchall()
        conn.close()
        if len(rows):
            for row in rows:
                print row
        return rows
        # 写入附件信息到文件
    def writeContactsInfoToFile(self,contactsInfo):
        #attachInfo = getContactsInfo(self.contactDB)
        f = codecs.open("%s-Contacts.txt" % self.userName,"w","utf-8")
        for eachContact in contactsInfo:
            for eachInfo in eachContact:
                text = unicode(str(eachInfo)+'\n'+smallSplit+"\n", "utf-8")
                f.write(text)
            f.write(largeSplit+u'\n')
    

    #正式工作流程  
    def mainHandle(self):
        #contactInfo = getContactsInfo(self.contactDB)
        mailInfo = self.getMailInfo()
        self.writeMailInfoToFile(mailInfo)
        self.getAttachInfo()
        #self.writeMailInfoToSingleFile(mailInfo)
        contactsInfo = self.getContactsInfo()
        self.writeContactsInfoToFile(contactsInfo)
        print "game over"

if __name__ == "__main__":
    if len(sys.argv)!=2:
        usage()
        exit()
    try:
        workDir = sys.argv[1]
        ne  =  netEaseMailMaster(workDir)
        ne.printInfo()
        ne.mainHandle()
    except Exception,e:
        print e
        
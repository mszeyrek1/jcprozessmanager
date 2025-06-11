package com.example.demo;

public class prozessinfo {
    private String pid;
    private String ppid;
    private String name;
    private String state;
    private String vmsize;
    private String rss;
    private String cputime;

    public String getPid() {return pid;}
    public void setPid(String pid) {
        this.pid= pid;
    }

    public String getPpid() {return ppid;}
    public void setPpid(String ppid) {
        this.ppid = ppid;
    }

    public String getName() {return name;}
    public void setName(String name) {
        this.name= name;
    }

    public String getState() {return state;}
    public void setState(String state) {
        this.state= state;
    }

    public String getVmsize() {return vmsize;}
    public void setVmsize(String vmsize) {
        this.vmsize= vmsize;
    }

    public String getRss() {return rss;}
    public void setRss(String rss) {
        this.rss = rss;
    }

    public String getCputime() {return cputime;}
    public void setCputime(String cputime) {
        this.cputime= cputime;
    }
}

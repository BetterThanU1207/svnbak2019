package com.aliyun.openservice.iot.as.bridge.demo.core;

import java.io.File;
import java.util.List;

/**
 * Created by Administrator on 2019/11/8.
 */
public class TerminalsConfMgr {

    public TerminalsConfMgr()
    {
            /*
            // 此处取项目路径 + 传入的路径,改路径获取不到文件
            // 如果要获取文件需要传入 src/main/resources/log4j2.xml

            //下面四种情况取编译后target\classes 目录下的文件
            // File 形式
            File file = new File(BootApplication.class.getClassLoader().getResource(filepath).getFile());
            System.out.println(file.getAbsolutePath());
            // InputStream 形式
            InputStream inputStream = BootApplication.class.getClassLoader().getResourceAsStream(filepath);
            System.out.println(inputStream);
            // URL 形式
            URL url = BootApplication.class.getClassLoader().getResource(filepath);
            System.out.println(url);
            // URI 形式
            URI uri = BootApplication.class.getClassLoader().getResource(filepath).toURI();
            File uriFile = new File(uri);
            System.out.println(uriFile.getAbsolutePath());
            */
    }
    /**
     * version : 1.0
     * description :
     * masters : [{"masterId":"","description":"","slaves":[{"slaveId":"","description":"","address":0,"type":1,"params":[{"paramName":"","description":"","address":0,"bytes":2}]}]}]
     */

    private String version;
    private String description;
    private List<MasterBean> masters;

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public List<MasterBean> getMasters() {
        return masters;
    }

    public void setMasters(List<MasterBean> masters) {
        this.masters = masters;
    }

    //查找指定主站下的所有从站信息
    public List<MasterBean.SlaveBean> getSlavesByMaster(String identity)
    {
        for(MasterBean m:masters)
        {
            if(m.getMasterId().equalsIgnoreCase(identity))
                return m.getSlaves();
        }
        return null;
    }

    public static class MasterBean {
        /**
         * masterId :
         * description :
         * slaves : [{"slaveId":"","description":"","address":0,"type":1,"params":[{"paramName":"","description":"","address":0,"bytes":2}]}]
         */

        private String masterId;
        private String description;
        private List<SlaveBean> slaves;

        public String getMasterId() {
            return masterId;
        }

        public void setMasterId(String masterId) {
            this.masterId = masterId;
        }

        public String getDescription() {
            return description;
        }

        public void setDescription(String description) {
            this.description = description;
        }

        public List<SlaveBean> getSlaves() {
            return slaves;
        }

        public void setSlaves(List<SlaveBean> slaves) {
            this.slaves = slaves;
        }

        public static class SlaveBean {
            /**
             * slaveId :
             * description :
             * address : 0
             * type : 1
             * params : [{"paramName":"","description":"","address":0,"bytes":2}]
             */

            private String slaveId;
            private String description;
            private int address;
            private int interval;
            private int dn;
            private int type;
            private List<ParamBean> params;

            public String getSlaveId() {
                return slaveId;
            }
            public void setSlaveId(String slaveId) {
                this.slaveId = slaveId;
            }

            public String getDescription() {
                return description;
            }
            public void setDescription(String description) {
                this.description = description;
            }

            public int getAddress() {
                return address;
            }
            public void setAddress(int address) {
                this.address = address;
            }

            public int getInterval(){return interval;}
            public void setInterval(int interval){this.interval = interval;}

            public int getDn(){return dn;}
            public void setDn(int dn){this.dn = dn;}

            public int getType() {
                return type;
            }
            public void setType(int type) {
                this.type = type;
            }

            public List<ParamBean> getParams() {
                return params;
            }
            public void setParams(List<ParamBean> params) {
                this.params = params;
            }

            public static class ParamBean {
                /**
                 * paramName :
                 * description :
                 * address : 0
                 * bytes : 2
                 */

                private String paramName;
                private String description;
                private int address;
                private int bytes;

                public String getParamName() {
                    return paramName;
                }

                public void setParamName(String paramName) {
                    this.paramName = paramName;
                }

                public String getDescription() {
                    return description;
                }

                public void setDescription(String description) {
                    this.description = description;
                }

                public int getAddress() {
                    return address;
                }

                public void setAddress(int address) {
                    this.address = address;
                }

                public int getBytes() {
                    return bytes;
                }

                public void setBytes(int bytes) {
                    this.bytes = bytes;
                }
            }
        }
    }
}

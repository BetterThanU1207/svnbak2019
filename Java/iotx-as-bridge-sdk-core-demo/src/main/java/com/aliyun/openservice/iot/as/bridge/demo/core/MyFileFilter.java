package com.aliyun.openservice.iot.as.bridge.demo.core;

import org.springframework.util.StringUtils;

import java.io.File;
import java.io.FileFilter;

class MyFileFilter
  implements FileFilter
{
  private String fileExtName;

  public MyFileFilter(String fileExtName)
  {
    this.fileExtName = fileExtName;
  }

  public boolean accept(File pathname) {
    return StringUtils.endsWithIgnoreCase(pathname.getName(), this.fileExtName);
  }
}
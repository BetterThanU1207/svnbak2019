package com.aliyun.openservice.iot.as.bridge.demo.core;

import org.springframework.util.StringUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.List;

public class ResourceUtils
{
  public static List<String> listFilePathFromResource(String resource, String fileExtName)
  {
    List filePaths = new ArrayList();

    fileExtName = "." + fileExtName.toLowerCase();

    if (StringUtils.endsWithIgnoreCase(resource, fileExtName)) {
      InputStream is = getInputStream(resource);
      if (is != null)
        filePaths.add(resource);
    } else {
      File[] arrayOfFile1;
      URL url = getResource(resource);
      if (url == null)
        return filePaths;

      String path = url.getFile();
      try {
        path = URLDecoder.decode(path, "UTF-8");
      } catch (UnsupportedEncodingException localUnsupportedEncodingException) {
      }
      File file = new File(path);
      if (!(file.isDirectory())) {
        return filePaths;
      }

      File[] xmlFiles = file.listFiles(new MyFileFilter(fileExtName));

      int j = (arrayOfFile1 = xmlFiles).length; for (int i = 0; i < j; ++i) { File xmlFile = arrayOfFile1[i];
        filePaths.add(resource + xmlFile.getName());
      }
    }

    return filePaths;
  }

  public static URL getResource(String name) {
    if (StringUtils.isEmpty(name))
      return null;

    ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
    URL url = null;

    if (name.indexOf(":/") > -1)
      try {
        url = new URL(name);
      }
      catch (Exception localException1) {
      }
    if (url == null)
      try {
        url = classLoader.getResource(name);
      }
      catch (Exception localException2) {
      }
    if (url == null)
      try {
        url = classLoader.getResource('/' + name);
      }
      catch (Exception localException3) {
      }
    if (url == null)
      try {
        url = classLoader.getResource("META-INF/" + name);
      }
      catch (Exception localException4) {
      }
    return url;
  }

  public static InputStream getInputStream(String name) {
    if (StringUtils.isEmpty(name))
      return null;

    InputStream is = null;

    URL url = getResource(name);
    if (url != null)
      try {
        is = url.openStream();
      } catch (IOException localIOException) {
      }
    else try {
        is = new FileInputStream(name);
      }
      catch (FileNotFoundException localFileNotFoundException) {
      }
    return is;
  }
}
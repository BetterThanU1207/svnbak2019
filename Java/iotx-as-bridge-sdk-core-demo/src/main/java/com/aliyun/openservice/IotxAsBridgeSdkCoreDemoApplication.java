package com.aliyun.openservice;

import com.aliyun.openservice.iot.as.bridge.demo.core.BridgeBasicV1;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class IotxAsBridgeSdkCoreDemoApplication {

	public static void main(String[] args) {
		SpringApplication.run(IotxAsBridgeSdkCoreDemoApplication.class, args);
		try {
			BridgeBasicV1.main(new String[]{});
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

}

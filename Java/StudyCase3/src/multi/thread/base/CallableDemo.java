package multi.thread.base;

import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;

class MyThread3 implements Callable<String> {
	@Override
	public String call() throws Exception {
		// TODO 自动生成的方法存根
		for (int i = 0; i < 10; i++) {
			System.out.println("**********线程执行、x=" + i);			
		}
		return "线程执行完毕。";
	}
}

public class CallableDemo {

	public static void main(String[] args) throws Exception{
		// TODO 自动生成的方法存根
		FutureTask<String> task = new FutureTask<>(new MyThread3());
		new Thread(task).start();
		System.out.println("【线程返回数据】" + task.get());
	}

}

/**

项目名称：StudyCase3
类名称：CallableAnswer
类描述：
创建人：Zoey
创建时间：2019年12月10日 下午3:30:28
@version
*/

package multi.thread.cases;

import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;

class MyThread implements Callable<String> {
	private boolean flag=false;	//抢答处理
	@Override
	public String call() throws Exception {
		// TODO 自动生成的方法存根
		synchronized (this) {	//数据同步
			if (this.flag == false) {	//抢答成功
				this.flag = true;
				return Thread.currentThread().getName() + "抢答成功！";
			} else {
				return Thread.currentThread().getName() + "抢答失败！";
			}
		}
	}
}

public class CallableAnswer {

	public static void main(String[] args) throws Exception {
		// TODO 自动生成的方法存根
		MyThread mt = new MyThread();
		FutureTask<String> task = new FutureTask<String>(mt);
		new Thread(task, "竞赛者A").start();
		new Thread(task, "竞赛者B").start();
		new Thread(task, "竞赛者C").start();
		System.out.println(task.get());
	}

}

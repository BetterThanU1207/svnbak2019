/**

项目名称：StudyCase3
类名称：NumAddSub
类描述：四个线程，两个加数字，两个减数字
创建人：Zoey
创建时间：2019年12月10日 上午11:37:42
@version	运行结果只能是0 1 -1循环出现，才合理，目前还有bug2019年12月10日15:14:35
*/

package multi.thread.cases;

public class NumAddSub {
	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		Resource res = new Resource();
		AddThread at = new AddThread(res);
		SubThread st = new SubThread(res);
		new Thread(at, "加法线程 - A").start();
		new Thread(at, "加法线程 - B").start();
		new Thread(st, "减法线程 - X").start();
		new Thread(st, "减法线程 - Y").start();
	}

}

class Resource {	//定义一个操作的资源
	private int num = 0;		//要进行加减操作的数据
	private boolean flag = true;	//加减的切换
	//flag=true：可以加不可以减
	//flag=false：可以减不可以加
	public synchronized void add() throws Exception {	//执行加法
		if (this.flag == false) {	//现在执行减法，加法需等待
			super.wait();
		}
		Thread.sleep(100);
		this.num ++;
		System.out.println("【加法操作 - " + Thread.currentThread().getName() + "】 num = " + this.num);
		this.flag = false;
		super.notifyAll();//唤醒全部等待线程
	}
	public synchronized void sub() throws Exception {	//执行减法
		if (this.flag == true) {	//现在执行加法，减法需等待
			super.wait();
		}
		Thread.sleep(100);
		this.num --;
		System.out.println("【减法操作 - " + Thread.currentThread().getName() + "】 num = " + this.num);
		this.flag = true;
		super.notifyAll();//唤醒全部等待线程
	}
}

class AddThread implements Runnable {
	private Resource resource;

	public AddThread(Resource resource) {
		// TODO 自动生成的构造函数存根
		this.resource = resource;
	}
	@Override
	public void run() {
		// TODO 自动生成的方法存根
		for (int i = 0; i < 50; i++) {
			try {
				this.resource.add();
			} catch (Exception e) {
				// TODO 自动生成的 catch 块
				e.printStackTrace();
			}
		}
	}
}

class SubThread implements Runnable {
	private Resource resource;

	public SubThread(Resource resource) {
		// TODO 自动生成的构造函数存根
		this.resource = resource;
	}
	@Override
	public void run() {
		// TODO 自动生成的方法存根
		for (int i = 0; i < 50; i++) {
			try {
				this.resource.sub();
			} catch (Exception e) {
				// TODO 自动生成的 catch 块
				e.printStackTrace();
			}
		}
	}
}


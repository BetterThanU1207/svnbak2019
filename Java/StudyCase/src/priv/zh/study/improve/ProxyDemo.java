/**

项目名称：StudyCase
类名称：ProxyDemo
类描述：代理设计模式
创建人：Zoey
创建时间：2019年12月4日 下午2:38:57
@version
*/

package priv.zh.study.improve;

interface IEat {
	public void get();
}

class EatReal implements IEat {
	@Override
	public void get() {
		// TODO 自动生成的方法存根
		System.out.println("【真是主题】得到一份食物");
	}
}

class EatProxy implements IEat { // 服务代理
	private IEat eat; // 为吃而服务

	public EatProxy(IEat eat) { // 一定要有一个代理项
		// TODO 自动生成的构造函数存根
		this.eat = eat;
	}

	@Override
	public void get() {
		// TODO 自动生成的方法存根
		this.prepare();
		this.eat.get();
		this.clear();
	}

	public void prepare() {
		System.out.println("【代理主题】准备食材。");
	}

	public void clear() {
		System.out.println("【代理主题】清洁。");
	}
}

public class ProxyDemo {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		IEat eat = new EatProxy(new EatReal());
		eat.get();
	}

}

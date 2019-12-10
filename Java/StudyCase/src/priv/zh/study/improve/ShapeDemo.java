/**

项目名称：StudyCase
类名称：SharpDemo
类描述：抽象类+工厂类
创建人：Zoey
创建时间：2019年12月4日 上午11:31:12
@version
*/

package priv.zh.study.improve;

abstract class AbstractShape {
	public abstract double area();

	public abstract double perimeter();
}
//同一子包不能类名相同
class Circular2 extends AbstractShape {
	private double radius;

	public Circular2(double radius) {
		// TODO 自动生成的构造函数存根
		this.radius = radius;
	}

	@Override
	public double area() {
		// TODO 自动生成的方法存根
		return 3.14 * this.radius * this.radius;
	}

	@Override
	public double perimeter() {
		// TODO 自动生成的方法存根
		return 2 * 3.14 * this.radius;
	}
}

class Rectangle extends AbstractShape {
	private double length;
	private double width;

	public Rectangle(double length, double width) {
		// TODO 自动生成的构造函数存根
		this.length = length;
		this.width = width;
	}

	@Override
	public double area() {
		// TODO 自动生成的方法存根
		return this.length * this.width;
	}

	@Override
	public double perimeter() {
		// TODO 自动生成的方法存根
		return 2 * (this.length + this.width);
	}
}

class Factory2 {
	public static AbstractShape getInstance(String className, double ... args) {
		if ("Circular".equalsIgnoreCase(className)) {
			return new Circular2(args[0]);
		} else if ("Rectangle".equalsIgnoreCase(className)) {
			return new Rectangle(args[0], args[1]);
		} else {
			return null;
		}
	}
}

public class ShapeDemo {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		AbstractShape ara = Factory2.getInstance("Circular", 1.1);
		AbstractShape arb = Factory2.getInstance("Rectangle", 1.1, 2.2);
		System.out.println("圆形面积：" + ara.area() + "圆形周长：" + ara.perimeter());
		System.out.println("矩形面积：" + arb.area() + "矩形周长：" + arb.perimeter());
	}

}

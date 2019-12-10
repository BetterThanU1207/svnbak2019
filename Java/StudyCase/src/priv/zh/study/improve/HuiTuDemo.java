/**

项目名称：StudyCase
类名称：SharpDemo
类描述：接口类+工厂类
创建人：Zoey
创建时间：2019年12月4日 上午11:31:12
@version
*/
package priv.zh.study.improve;

interface IGraphical { // 定义绘图标准
	public void paint(); // 绘图
}

class Point {	//坐标
	private double x;
	private double y;
	public Point(double x, double y) {
		this.x = x;
		this.y = y;		
	}
	public double getX() {
		return this.x;
	}
	public double getY() {
		return this.y;
	}
}

class Triangle implements IGraphical {
	private Point[] x; // 保存第一条边的坐标
	private Point[] y; // 保存第二条边的坐标
	private Point[] z; // 保存第三条边的坐标

	public Triangle(Point[] x, Point[] y, Point[] z) {
		// TODO 自动生成的构造函数存根
		this.x = x;
		this.y = y;
		this.z = z;
	}

	@Override
	public void paint() {
		System.out.println("开始绘制第一条边，开始坐标：[" + this.x[0].getX() + "," + this.x[0].getY()
				+ "],结束坐标：" + this.x[1].getX() + "," + this.x[1].getY());
		System.out.println("开始绘制第二条边，开始坐标：[" + this.y[0].getX() + "," + this.y[0].getY()
				+ "],结束坐标：" + this.y[1].getX() + "," + this.y[1].getY());
		System.out.println("开始绘制第三条边，开始坐标：[" + this.z[0].getX() + "," + this.z[0].getY()
				+ "],结束坐标：" + this.z[1].getX() + "," + this.z[1].getY());
	}
}

class Circular implements IGraphical {
	private double radius;

	public Circular(double radius) {
		// TODO 自动生成的构造函数存根
		this.radius = radius;
	}
	@Override
	public void paint() {
		// TODO 自动生成的方法存根
		System.out.println("以半径为“" + this.radius + "”绘制圆形。");
	}
}

class Factory {	//工厂类
	public static IGraphical getInstance (String className, double ... args) {
		if ("triangle".equalsIgnoreCase(className)) {
			return new Triangle(
					new Point[] {
							new Point(args[0], args[1]),
							new Point(args[2], args[3])
							},
					new Point[] {
							new Point(args[4], args[5]),
							new Point(args[6], args[6])
							},
					new Point[] {
							new Point(args[8], args[9]),
							new Point(args[10], args[11])
							}
					);
		} else if ("circular".equalsIgnoreCase(className)) {
			return new Circular(args[0]);
		} else {
			return null;
		}
	}
}

public class HuiTuDemo {
	public static void main (String args[]) {
		IGraphical iga = Factory.getInstance("triangle", 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.10, 11.11, 12.12);
		iga.paint();
		IGraphical igb = Factory.getInstance("circular", 12.34);
		igb.paint();
	}
}

package priv.zh.study.before;

//数据表与简单Java类映射转换
/*
 * 一个部门有多个雇员
 * 一个雇员属于一个部门
 * 一个雇员有一个领导 
 */
/*
 * 根据部门信息获取以下内容
 * 	一个部门的完整信息
 * 	一个部门之中所有雇员的完整信息
 * 	一个雇员对应的领导的信息
 * 根据雇员信息获得以下内容
 * 	一个雇员所在部门信息
 * 	一个雇员对应的领导信息
 */
class Dept {
	private long deptno;
	private String dname;
	private String loc;
	private Emp emps[]; // 多个雇员信息

	public Dept(long deptno, String dname, String loc) {
		this.deptno = deptno;
		this.dname = dname;
		this.loc = loc;
	}

	public void setEmps(Emp[] emps) {
		this.emps = emps;
	}

	public Emp[] getEmps() {
		return this.emps;
	}

	// setter、getter、无参构造略
	public String getInfo() {
		return "【部门信息】部门编号 = " + this.deptno + "、部门名称 = " + this.dname + "、部门位置 = " + this.loc;
	}
}

class Emp {
	private long empno;
	private String ename;
	private String job;
	private double sal;
	private double comm;
	private Dept dept; // 所属部门
	private Emp mgr; // 所属领导

	public Emp(long empno, String ename, String job, double sal, double comm) {
		this.empno = empno;
		this.ename = ename;
		this.job = job;
		this.sal = sal;
		this.comm = comm;
	}

	// setter、getter、无参构造略
	public String getInfo() {
		return "【雇员信息】部门编号 = " + this.empno + "、雇员姓名名称 = " + this.ename + "、雇员职位 = " + this.job + "、基本工资 = " + this.sal
				+ "、佣金 = " + this.comm;
	}

	public void setDept(Dept dept) {
		this.dept = dept;
	}

	public void setMgr(Emp mgr) {
		this.mgr = mgr;
	}

	public Dept getDept() {
		return this.dept;
	}

	public Emp getMgr() {
		return this.mgr;
	}
}

public class JavaCompany {
	public static void main(String[] args) {
		// 第一步：根据关心进行类的定义
		Dept dept = new Dept(10, "财务部", "上海");
		Emp empA = new Emp(7396, "SMITH", "CLERK", 800.00, 0.0);
		Emp empB = new Emp(7566, "FORD", "MANAGER", 2450.00, 0.0);
		Emp empC = new Emp(7839, "KING", "PRESIDENT", 5000.00, 0.0);
		// 需要为对象进行关联设置
		empA.setDept(dept); // 设置雇员与部门的关联
		empB.setDept(dept);
		empC.setDept(dept);
		empA.setMgr(empB); // 设置雇员与领导的关联
		empB.setMgr(empC);

		dept.setEmps(new Emp[] { empA, empB, empC });
		// 第二步：根据关系获取数据
		System.out.println(dept.getInfo());// 部门信息
		for (int x = 0; x < dept.getEmps().length; x++) {
			System.out.println("\t|-" + dept.getEmps()[x].getInfo());
			if (dept.getEmps()[x].getMgr() != null) {
				System.out.println("\t\t|-所属领导：" + dept.getEmps()[x].getMgr().getInfo());
			}
			System.out.println("---------------------------------------------------");
			System.out.println(empB.getDept().getInfo());// 根据雇员获取部门信息
			System.out.println(empB.getMgr().getInfo());// 根据雇员获取部门信息
		}
	}
}

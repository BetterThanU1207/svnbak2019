package generics.factory;

import generics.factory.car.Audi;
import generics.factory.car.Benz;
import generics.factory.car.Car;
import generics.factory.factory.Factory;
import generics.factory.factory.GenericFactory;
import generics.factory.tv.Sharp;
import generics.factory.tv.Sony;
import generics.factory.tv.TV;

public class MyProgram {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		Factory<Car> carFactory = new GenericFactory<>();
		Factory<TV> TVFactory = new GenericFactory<>();
		
		Car benzCar = carFactory.getProduct(Benz.class);
		Car audiCar = carFactory.getProduct(Audi.class);
		benzCar.run();
		audiCar.run();
		
		TV sharpTv = TVFactory.getProduct(Sharp.class);
		TV sonTv = TVFactory.getProduct(Sony.class);
		sharpTv.play();
		sonTv.play();
		
	}

}

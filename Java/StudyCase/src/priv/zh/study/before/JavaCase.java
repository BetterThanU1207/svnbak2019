package priv.zh.study.before;

class Address {
	private String country;
	private String province;
	private String city;
	private String street;
	private String zipcode;

	public Address() {
	}

	public Address(String country, String province, String city, String street, String zipcode) {
		this.country = country;
		this.province = province;
		this.city = city;
		this.street = street;
		this.zipcode = zipcode;
	}

	public String getInfo() {
		return "国家：" + this.country + "、省份：" + this.province + "、城市：" + this.city + "、街道：" + this.street + "、邮编："
				+ this.zipcode;
	}

	public void setCountry(String country) {
		this.country = country;
	}

	public void setProvince(String province) {
		this.province = province;
	}

	public void setCity(String city) {
		this.city = city;
	}

	public void setStreet(String street) {
		this.street = street;
	}

	public void setZipCode(String zipcode) {
		this.zipcode = zipcode;
	}

	public String getCountry() {
		return this.country;
	}

	public String getProvince() {
		return this.province;
	}

	public String getCity() {
		return this.city;
	}

	public String getStreet() {
		return this.street;
	}

	public String getZipcode() {
		return this.zipcode;
	}
}

public class JavaCase {
	public static void main(String[] args) {
		System.out.println(new Address("中国", "山东", "济宁", "金宇", "10001").getInfo());
	}
}

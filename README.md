## Glide: Programming Language geared towards data transformations

Glide is a statically (and dynamically) typed language designed to make reading and writing data transformations easier.

# How to use:

To compile your own version of Glide, simply change the "Type" in main.cpp to "INTERP", and compile using "C/C++: clang++ build interpreter". Note: currently only tested on Mac, but try your luck with the Windows build too. It may throw some errors that should be easy to iron out.

If you want to build in developer mode, leave "Type" as "DEV" and compile using "C/C++: clang++ build and debug active file".

For both instances, you need to be in main.cpp for compilation to work.

If you compile the interpreter, the executable is found in "bin/build/interp".

If you compile in debug mode, the executable is found in "bin/build/dev".

# Examples

## Basic data transformation:
```
x = 1..100 
	>> map[x => x * 2.5]
	>> filter[x => x > 125]
	>> reduce[+]

print[x]

Output: 9187.500000
```

## Multiple dispatch + refinement types:
```
PosInt :: type = x::int => x > 0

Deposit :: type = {
    amount: PosInt
}
Withdrawal :: type = {
    amount: PosInt
}
CheckBalance :: type

applyAction = [action::Deposit] => "Depositing $" + action.amount
applyAction = [action::Withdrawal] => "Withdrawing $" + action.amount
applyAction = [action::CheckBalance] => "Checking balance..."

d :: Withdrawal = {
    amount: 35
}

res = applyAction[d]

// Output: "Withdrawing $35"
```

## Pattern matching:
```
pop_f = ls::[] => {
    match[ls] {
        []: []
        [first ...rest]: [first rest]
        []
    }
}

res = 1..10 >> pop_f

// Output: [1 [2 3 4 5 6 7 8 9]]
```

```
Cash :: type = {
    amount: int
}
PayPal :: type = {
    amount: int
    email: string
}
CreditCard :: type = {
    amount: int
    cardNumber: string
    securityCode: string
}

PaymentMethod = Cash | PayPal | CreditCard

describePayment = [method::PaymentMethod] => {
    match[method] {
        Cash: "CASH - $" + method.amount
        PayPal: "PAYPAL - $" + method.amount + " - " + method.email
        CreditCard: "CREDIT - $" + method.amount + " - " + method.cardNumber + " - " + method.securityCode
        "Undefined payment method"
    }
}

p1 :: PaymentMethod = {
    amount: 23556
    cardNumber: "838128384"
    securityCode: "8372"
}

p2 :: PaymentMethod = {
    amount: 42882
}

p3 :: PaymentMethod = {
    amount: 42882
    email: "blah@test.com"
}

describePayment[p2]

// Output: "CASH - $42882"
```

## Tagged unions + pattern matching:
```
Animal = Dog::type | Cat::type | Bird::type

p = [bool | Animal]

x :: p = [true Bird]

categoryId = match[x] {
    [true {Dog}]: 1
    [true {Cat}]: 2
    [true {Bird}]: 3
    [false {Dog | Cat}]: 4
    [false {Bird}]: 5
    (-1)
}

categoryId >> print

// Output: 3
```

# Data tranformation API

## This is an example of data transformation using the csv module:

```
csv = import["imports/csv.gl"]

employees = csv.load["src/data/employees.csv" schema: { 
	id: int
	age: int 
	salary: float
	is_manager: bool
	departmentId: int
}]

departments = csv.load["src/data/departments.csv" schema: {
	id: int
}]

extract_schema = {
	id: id::int => "EMP_" + id
	name: name::string => name
	salary: salary::int => salary
	is_manager: is_manager::bool => is_manager
	department: obj => csv.ref[departments "id" obj.departmentId]
}

stage_1_schema = {
	salary: [salary::int obj] => match[obj] {
		{ is_manager: true }: salary * 1.35
		salary * 0.85
	}
}

stage_2_schema = {
	tax: obj => match[obj] {
		{ salary: x => x < 100000 }: 10
		14.5
	}
	employeeID: obj => "00" + obj.id.split["_"].last
}

employees 
>> csv.extract[extract_schema]
>> (t1=)
>> csv.reshape[stage_1_schema]
>> (t2=)
>> csv.reshape[stage_2_schema]
>> (t3=)
>> csv.group_by["department" csv.COUNT[]]
>> (t4=) 
>> (x => t3)
>> csv.group_by["department" csv.AVG["salary"]]
>> (t5=)
```

## Employees.csv

```
id,name,age,location,salary,is_manager,departmentId
1,Allan Jones,32,Sydney,100000.00,true,1
2,Allan Jones,25,Melbourne,150000.00,false,1
3,James Wright,23,Brisbane,89000.00,false,2
4,Haley Smith,25,Bondi,78000.00,true,2
5,Jessica Mayfield,27,Greenacre,120000.00,true,2
6,Jessica Rogers,22,Surry Hills,68000.00,false,3
7,Eric Ericson,24,Camperdown,92000.00,false,4
```

## Departments.csv

```
id,name
1,Sales
2,Marketing
3,Engineering
4,Analytics
```

## Output of t3:

```
[ {
  is_manager: true
  name: Allan Jones
  salary: 135000.000000
  id: EMP_1
  department: Sales
  employeeID: 001
  tax: 14.500000
} {
  is_manager: false
  name: Allan Jones
  salary: 127500.000000
  id: EMP_2
  department: Sales
  employeeID: 002
  tax: 14.500000
} {
  is_manager: false
  name: James Wright
  salary: 75650.000000
  id: EMP_3
  department: Marketing
  employeeID: 003
  tax: 10
} {
  is_manager: true
  name: Haley Smith
  salary: 105300.000000
  id: EMP_4
  department: Marketing
  employeeID: 004
  tax: 14.500000
} {
  is_manager: true
  name: Jessica Mayfield
  salary: 162000.000000
  id: EMP_5
  department: Marketing
  employeeID: 005
  tax: 14.500000
} {
  is_manager: false
  name: Jessica Rogers
  salary: 57800.000000
  id: EMP_6
  department: Engineering
  employeeID: 006
  tax: 10
} {
  is_manager: false
  name: Eric Ericson
  salary: 78200.000000
  id: EMP_7
  department: Analytics
  employeeID: 007
  tax: 10
} ]
```

## Explanation of the above code:

1- Import the csv module

2- Load the 2 pieces of data (employees and departments). Think of these as two tables in a database. The schema object is used to transform the types of the data, since csv data is all string based. This may or may not be useful once we load from a database, given that we may already know the types ahead of loading.

3- We define the extraction schema. This is the first stage of the pipeline. What we're doing here is extracting the relevant columns, but also with the option to transform that data as we extract (as shown in the id column). We can also create new columns here based on known data, as shown in the departments column. Any column not defined here is not extracted.

4- We then set up two other stages, which do the same thing as the extraction schema, except they only affect the columns defined in the schema. The rest of the columns are left intact.

5- We run the pipeline, starting with the extraction, and then the reshaping of the data. Note that we are saving each step of the transformation in its own variable for future reference (this is possible because we are piping the result of a transformation into a partial equal op, which then evaluates and saves the data).

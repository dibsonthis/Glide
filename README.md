## Glide: Programming Language geared towards data transformations

Glide is a statically (and dynamically) typed language designed to make reading and writing data transformations easier.

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

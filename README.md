## Glide: Programming Language geared towards data transformations

Glide is a statically (and dynamically) typed language designed to make reading and writing data transformations easier.

# Examples

```
x = 1..100 
	>> map[x => x * 2.5]
	>> filter[x => x > 125]
	>> reduce[+]

print[x]
```

```
PosInt :: type = x::int => x > 0

Deposit :: type = {
    amount: PosInt
}
Withdrawal :: type = {
    amount: PosInt
}
CheckBalance :: type = {}

applyAction = [action::Deposit] => "Depositing $" + action.amount
applyAction = [action::Withdrawal] => "Withdrawing $" + action.amount
applyAction = [action::CheckBalance] => "Checking balance..."

d :: Withdrawal = {
    amount: 35
}

res = applyAction[d]
```

```
pop_f = ls::[] => {
    match[ls] {
        []: []
        [first ...rest]: [first rest]
        []
    }
}

res = (1..10 >> pop_f >> (x => x.last) >> pop_f);
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

res = describePayment[p2]
```
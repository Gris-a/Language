OPERATOR(ADD, "plus",
{
    if(IS_NODE_ZERO(LEFT))
    {
        copy = __CPY(RIGHT);
    }
    else if(IS_NODE_ZERO(RIGHT))
    {
        copy = __CPY(LEFT);
    }
    break;
},
{
    return (lvalue + rvalue);
})

OPERATOR(SUB, "minus",
{
    if(IS_NODES_SAME_VAR(LEFT, RIGHT))
    {
        copy = __VAL(0);
    }
    else if(IS_NODE_ZERO(RIGHT))
    {
        copy = __CPY(LEFT);
    }
    break;
},
{
    return (lvalue - rvalue);
})

OPERATOR(MUL, "umnozhit",
{
    if(IS_NODE_ZERO(LEFT) ||
       IS_NODE_ZERO(RIGHT))
    {
        copy = __VAL(0);
    }
    else if(IS_NODE_ONE(LEFT))
    {
        copy = __CPY(RIGHT);
    }
    else if(IS_NODE_ONE(RIGHT))
    {
        copy = __CPY(LEFT);
    }
    break;
},
{
    return (lvalue * rvalue);
})

OPERATOR(DIV, "delit",
{
    if(IS_NODES_SAME_VAR(LEFT, RIGHT))
    {
        copy = __VAL(1);
    }
    else if(IS_NODE_ZERO(LEFT))
    {
        copy = __VAL(0);
    }
    else if(IS_NODE_ONE(RIGHT))
    {
        copy = __CPY(LEFT);
    }
    break;
},
{
    if(abs(rvalue) < M_ERR)
    {
        printf("Error: Division by zero.\n");
        return NAN;
    }
    return (lvalue / rvalue);
})

OPERATOR(POW, "VoZVesti",
{
    if(IS_NODE_ONE(LEFT) ||
      IS_NODE_ZERO(RIGHT))
    {
        copy = __VAL(1);
    }
    else if(IS_NODE_ZERO(LEFT))
    {
        copy = __VAL(0);
    }
    else if(IS_NODE_ONE(RIGHT))
    {
        copy = __CPY(LEFT);
    }
    break;
},
{
    if((abs(lvalue) < M_ERR) && (abs(rvalue) < M_ERR))
    {
        printf("Error: 0^0\n");
        return NAN;
    }
    else if((abs(lvalue) < M_ERR) && rvalue < 0)
    {
        printf("Error: Division by zero.\n");
        return NAN;
    }
    else if(abs(floor(rvalue) - rvalue) < M_ERR)
    {
        if(rvalue < 0) return FastPow(1 / lvalue, (long long)rvalue * (-1));
        else           return FastPow(lvalue    , (long long)rvalue);
    }
    else
    {
        if(lvalue < 0)
        {
            printf("Error: neg base.\n");
            return NAN;
        }
        return pow(lvalue, rvalue);
    }
})

OPERATOR(LESS, "menshe",
{
    if(IS_NODES_SAME_VAR(LEFT, RIGHT))
    {
        copy = __VAL(0);
    }
    break;
},
{
    return (lvalue < rvalue) ? 1 : 0;
})

OPERATOR(ABOVE, "bolshe",
{
    if(IS_NODES_SAME_VAR(LEFT, RIGHT))
    {
        copy = __VAL(0);
    }
    break;
},
{
    return (lvalue > rvalue) ? 1 : 0;
})

OPERATOR(LESSEQ, "mensheraVno",
{
    if(IS_NODES_SAME_VAR(LEFT, RIGHT))
    {
        copy = __VAL(1);
    }
    break;
},
{
    return (lvalue - rvalue < M_ERR) ? 1 : 0;
})

OPERATOR(ABOVEEQ, "bolsheraVno",
{
    if(IS_NODES_SAME_VAR(LEFT, RIGHT))
    {
        copy = __VAL(1);
    }
    break;
},
{
    return (rvalue - lvalue < M_ERR) ? 1 : 0;
})

OPERATOR(EQ, "raVnoraVno",
{
    if(IS_NODES_SAME_VAR(LEFT, RIGHT))
    {
        copy = __VAL(1);
    }
    break;
},
{
    return (DBL_EQ(lvalue, rvalue)) ? 1 : 0;
})

OPERATOR(NOTEQ, "neraVno",
{
    if(IS_NODES_SAME_VAR(LEFT, RIGHT))
    {
        copy = __VAL(0);
    }
    break;
},
{
    return (!DBL_EQ(lvalue, rvalue)) ? 1 : 0;
})

OPERATOR(AND, "i",
{
    if(IS_NODE_ZERO(LEFT) ||
       IS_NODE_ZERO(RIGHT))
    {
        copy = __VAL(0);
    }
    break;
},
{
    return (!(DBL_EQ(lvalue, 0) || DBL_EQ(rvalue, 0)));
})

OPERATOR(OR, "ili",
{
    if(IS_NODE_ONE(LEFT) ||
       IS_NODE_ONE(RIGHT))
    {
        copy = __VAL(1);
    }
    break;
},
{
    return (!(DBL_EQ(lvalue, 0) && DBL_EQ(rvalue, 0)));
})

OPERATOR(ASS, "raVno",
{
    break;
},
{
    printf("Error: Invalid operator.\n");

    return NAN;
})
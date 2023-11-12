Sameep Singh
Student ID
 
 - Any resource you used for assistance
        -- I used lecture slides and recorded lectures 
 
 - The state of your implementation – whether any functionality is missing or the assignment is complete
        -- According to my knowledge, the assignment is finished.
         Every feature is available, and my testing revealed no memory leaks.
 
 - A summary of your new hashing algorithm, explaining whether and how it meets its must, should and ideally goals
        -- How It Must Run:
    The hashByOddEven method creates a unique code for each word or set of characters.

    It works simply by checking each character in a word. If a character's number value is odd, the method adds it to a total; if it's even, it subtracts it.
    
    This process is very quick and doesn't require complex calculations, making it fast for creating these unique codes.   

    
    Goals Acheived -
    Efficiency in Creating Unique Codes: The hashByOddEven method is efficient at generating unique codes for different words or sets of characters. 
    By using a simple addition and subtraction approach based on whether a character’s value is odd or even, it creates distinct codes quickly.

    Minimized Collisions: One of the key strengths of this method is its ability to keep collisions to a minimum. Despite its simplicity, the approach of differentiating between odd and even character values helps in reducing the likelihood that different words will end up with the same code.

    Speed and Simplicity: The method operates swiftly due to its straightforward nature. This makes it an excellent choice for applications where speed is crucial, and complex calculations are unnecessary.
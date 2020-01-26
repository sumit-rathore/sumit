def threeProductSuggestions(numProducts, repository, customerQuery):
    # WRITE YOUR CODE HERE
#   result = []
#   for product in range(0, numProducts):
#       if repository[product].startswith(searchString):
#               result.append(repository[product])
#   print(*result)
    #searchString = customerQuery[:2]
    result = []
    for numChar in range(2, len(customerQuery)):
        searchString = customerQuery[:numChar]
        result.append(match(numProducts, repository, searchString))
        result.append(customerQuery)
    print(*result)
    pass

def match(numProducts, repository, searchString):
    result = []
    for product in range(0, numProducts):
        if repository[product].startswith(searchString):
                result.append(repository[product])
    result.sort()
    return result

def main():
    numP = 5
    repo = ['bags', 'baggage', 'banner', 'box', 'cloths']
    query = 'bags'
    threeProductSuggestions(numP, repo, query)

main()

export function getArrayAvg(array: number[] | undefined | null): number {
    if (!array){
        return 0
    }
    let sum: number = 0;
    const n: number = array.length
    array.forEach(
        (element: number, index: number) => {
        sum += element;
        }
    )
    return sum / n;
}

// converts an array of hourly measurments to daily measurments
export function hourlyToDaily(array: number[]): number[] {
    let n: number = array.length;
    const res: number[] = []
    let unit: number = 0;
     

    for (let i = 0; i < n; i++) {
        unit += array[i]

        // Every 24 hours, push the average
        if ((i + 1) % 24 === 0 || i === n - 1) {
            res.push(unit / Math.min(24, i % 24 + 1))
            // reset for next day
            unit = 0
        }
    }
    return res
}